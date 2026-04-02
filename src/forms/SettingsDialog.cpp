/*
obs-websocket
Copyright (C) 2016-2021 Stephane Lepin <stephane.lepin@gmail.com>
Copyright (C) 2020-2021 Kyle Manning <tt2468@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <QMessageBox>
#include <QDateTime>
#include <QSignalBlocker>
#include <QTime>
#include <QUrl>

#include <optional>
#include <obs-module.h>
#include <obs.hpp>

#include "SettingsDialog.h"
#include "../obs-websocket.h"
#include "../Config.h"
#include "../websocketserver/WebSocketServer.h"
#include "../websocketclient/WebSocketClient.h"
#include "../utils/Crypto.h"

QString GetToolTipIconHtml()
{
	bool lightTheme = QApplication::palette().text().color().redF() < 0.5;
	QString iconFile = lightTheme ? ":toolTip/images/help.svg" : ":toolTip/images/help_light.svg";
	QString iconTemplate = "<html> <img src='%1' style=' vertical-align: bottom; ' /></html>";
	return iconTemplate.arg(iconFile);
}

struct ParsedClientConnection {
	bool valid = false;
	bool useTls = false;
	std::string host;
	std::optional<uint16_t> port;
	const char *errorTitleKey = "OBSWebSocket.Settings.ClientHostInvalidTitle";
	const char *errorMessageKey = "OBSWebSocket.Settings.ClientHostInvalidMessage";
};

QString BuildClientConnectionInput(const std::string &host, uint16_t port, bool useTls)
{
	if (host.empty())
		return {};

	QString hostValue = QString::fromStdString(host);
	if (hostValue.contains(':') && !hostValue.startsWith('[') && !hostValue.endsWith(']'))
		hostValue = QString("[%1]").arg(hostValue);

	return QString("%1://%2:%3").arg(useTls ? "wss" : "ws", hostValue).arg(port);
}

ParsedClientConnection ParseClientConnectionInput(const QString &connectionInput)
{
	ParsedClientConnection parsed;
	QString trimmed = connectionInput.trimmed();
	if (trimmed.isEmpty())
		return parsed;

	QUrl endpoint(trimmed, QUrl::StrictMode);
	if (!endpoint.isValid()) {
		parsed.errorMessageKey = "OBSWebSocket.Settings.ClientConnectionInvalidMessage";
		return parsed;
	}

	QString scheme = endpoint.scheme().toLower();
	if (scheme != "ws" && scheme != "wss") {
		parsed.errorMessageKey = "OBSWebSocket.Settings.ClientConnectionInvalidSchemeMessage";
		return parsed;
	}

	if (endpoint.host().isEmpty()) {
		parsed.errorMessageKey = "OBSWebSocket.Settings.ClientHostInvalidMessage";
		return parsed;
	}

	if (!endpoint.userName().isEmpty() || !endpoint.password().isEmpty()) {
		parsed.errorMessageKey = "OBSWebSocket.Settings.ClientConnectionCredentialsMessage";
		return parsed;
	}

	QString path = endpoint.path();
	if ((path.size() > 1) || endpoint.hasQuery() || endpoint.hasFragment()) {
		parsed.errorMessageKey = "OBSWebSocket.Settings.ClientConnectionPathMessage";
		return parsed;
	}

	int parsedPort = endpoint.port(-1);
	if (parsedPort != -1) {
		if (parsedPort < 1 || parsedPort > 65534) {
			parsed.errorMessageKey = "OBSWebSocket.Settings.ClientConnectionPortMessage";
			return parsed;
		}
		parsed.port = static_cast<uint16_t>(parsedPort);
	}

	parsed.valid = true;
	parsed.useTls = (scheme == "wss");
	parsed.host = endpoint.host().toStdString();
	return parsed;
}

void SetClientPasswordVisible(Ui::SettingsDialog *ui, bool visible)
{
	ui->clientPasswordLineEdit->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
	ui->showClientPasswordButton->setText(
		obs_module_text(visible ? "OBSWebSocket.Settings.Hide" : "OBSWebSocket.Settings.Show"));
}

SettingsDialog::SettingsDialog(QWidget *parent)
	: QDialog(parent, Qt::Dialog),
	  ui(new Ui::SettingsDialog),
	  connectInfo(new ConnectInfo),
	  sessionTableTimer(new QTimer),
	  passwordManuallyEdited(false),
	  clientPasswordManuallyEdited(false)
{
	ui->setupUi(this);
	ui->websocketSessionTable->horizontalHeader()->resizeSection(3, 100); // Resize Session Table column widths
	ui->websocketSessionTable->horizontalHeader()->resizeSection(4, 100);

	// Ensure the window cannot be resized below its full content height.
	const int contentMinHeight = sizeHint().height();
	if (minimumHeight() < contentMinHeight)
		setMinimumHeight(contentMinHeight);
	if (height() < contentMinHeight)
		resize(width(), contentMinHeight);

	// Remove the ? button on dialogs on Windows
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	// Set the appropriate tooltip icon for the theme
	ui->clientWarningToolTipLabel->setText(GetToolTipIconHtml());
	ui->enableDebugLoggingToolTipLabel->setText(GetToolTipIconHtml());

	connect(sessionTableTimer, &QTimer::timeout, this, &SettingsDialog::FillSessionTable);
	connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &SettingsDialog::DialogButtonClicked);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(ui->enableWebSocketServerCheckBox, &QCheckBox::checkStateChanged, this, &SettingsDialog::UpdateServerUiState);
	connect(ui->enableAuthenticationCheckBox, &QCheckBox::checkStateChanged, this,
		&SettingsDialog::EnableAuthenticationCheckBoxChanged);
	connect(ui->enableWebSocketClientCheckBox, &QCheckBox::checkStateChanged, this, &SettingsDialog::UpdateClientUiState);
	connect(ui->clientAuthRequiredCheckBox, &QCheckBox::checkStateChanged, this, &SettingsDialog::UpdateClientUiState);
	connect(ui->clientAllowInsecureCheckBox, &QCheckBox::checkStateChanged, this, &SettingsDialog::UpdateClientUiState);
#else
	connect(ui->enableWebSocketServerCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::UpdateServerUiState);
	connect(ui->enableAuthenticationCheckBox, &QCheckBox::stateChanged, this,
		&SettingsDialog::EnableAuthenticationCheckBoxChanged);
	connect(ui->enableWebSocketClientCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::UpdateClientUiState);
	connect(ui->clientAuthRequiredCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::UpdateClientUiState);
	connect(ui->clientAllowInsecureCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::UpdateClientUiState);
#endif
	connect(ui->clientHostLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::UpdateClientUiState);
	connect(ui->generatePasswordButton, &QPushButton::clicked, this, &SettingsDialog::GeneratePasswordButtonClicked);
	connect(ui->generateClientPasswordButton, &QPushButton::clicked, this,
		&SettingsDialog::GenerateClientPasswordButtonClicked);
	connect(ui->showClientPasswordButton, &QPushButton::clicked, this, &SettingsDialog::ToggleClientPasswordVisibility);
	connect(ui->showConnectInfoButton, &QPushButton::clicked, this, &SettingsDialog::ShowConnectInfoButtonClicked);
	connect(ui->serverPasswordLineEdit, &QLineEdit::textEdited, this, &SettingsDialog::PasswordEdited);
	connect(ui->clientPasswordLineEdit, &QLineEdit::textEdited, this, &SettingsDialog::ClientPasswordEdited);
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
	delete connectInfo;
	delete sessionTableTimer;
}

void SettingsDialog::showEvent(QShowEvent *)
{
	auto conf = GetConfig();
	if (!conf) {
		blog(LOG_ERROR, "[SettingsDialog::showEvent] Unable to retreive config!");
		return;
	}

	if (conf->PortOverridden)
		ui->serverPortSpinBox->setEnabled(false);

	if (conf->PasswordOverridden) {
		ui->enableAuthenticationCheckBox->setEnabled(false);
		ui->serverPasswordLineEdit->setEnabled(false);
		ui->generatePasswordButton->setEnabled(false);
	}

	passwordManuallyEdited = false;
	clientPasswordManuallyEdited = false;

	RefreshData();

	sessionTableTimer->start(1000);
}

void SettingsDialog::hideEvent(QHideEvent *)
{
	if (sessionTableTimer->isActive())
		sessionTableTimer->stop();

	connectInfo->hide();
}

void SettingsDialog::ToggleShowHide()
{
	if (!isVisible())
		setVisible(true);
	else
		setVisible(false);
}

void SettingsDialog::RefreshData()
{
	auto conf = GetConfig();
	if (!conf) {
		blog(LOG_ERROR, "[SettingsDialog::RefreshData] Unable to retreive config!");
		return;
	}

	ui->enableWebSocketServerCheckBox->setChecked(conf->ServerEnabled);
	ui->enableSystemTrayAlertsCheckBox->setChecked(conf->AlertsEnabled);
	ui->enableDebugLoggingCheckBox->setChecked(conf->DebugEnabled);
	ui->serverPortSpinBox->setValue(conf->ServerPort);
	ui->enableAuthenticationCheckBox->setChecked(conf->AuthRequired);
	ui->serverPasswordLineEdit->setText(QString::fromStdString(conf->ServerPassword));

	ui->serverPasswordLineEdit->setEnabled(conf->AuthRequired);
	ui->generatePasswordButton->setEnabled(conf->AuthRequired);

	ui->enableWebSocketClientCheckBox->setChecked(conf->ClientEnabled);
	ui->clientHostLineEdit->setText(
		BuildClientConnectionInput(conf->ClientHost, conf->ClientPort.load(), conf->ClientUseTls.load()));
	ui->clientAllowInsecureCheckBox->setChecked(conf->ClientAllowInsecure);
	ui->clientAllowInvalidCertCheckBox->setChecked(conf->ClientAllowInvalidCert);
	ui->clientAuthRequiredCheckBox->setChecked(conf->ClientAuthRequired);
	ui->clientPasswordLineEdit->setText(QString::fromStdString(conf->ClientPassword));
	SetClientPasswordVisible(ui, false);

	UpdateServerUiState();
	UpdateClientUiState();
	UpdateClientStatus();

	FillSessionTable();
}

void SettingsDialog::DialogButtonClicked(QAbstractButton *button)
{
	if (button == ui->buttonBox->button(QDialogButtonBox::Ok)) {
		SaveFormData();
	} else if (button == ui->buttonBox->button(QDialogButtonBox::Apply)) {
		SaveFormData();
	}
}

void SettingsDialog::SaveFormData()
{
	auto conf = GetConfig();
	if (!conf) {
		blog(LOG_ERROR, "[SettingsDialog::SaveFormData] Unable to retreive config!");
		return;
	}

	if (ui->serverPasswordLineEdit->text().length() < 6) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(obs_module_text("OBSWebSocket.Settings.Save.PasswordInvalidErrorTitle"));
		msgBox.setText(obs_module_text("OBSWebSocket.Settings.Save.PasswordInvalidErrorMessage"));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.exec();
		return;
	}

	// Show a confirmation box to the user if they attempt to provide their own password
	if (passwordManuallyEdited && (conf->ServerPassword != ui->serverPasswordLineEdit->text().toStdString())) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(obs_module_text("OBSWebSocket.Settings.Save.UserPasswordWarningTitle"));
		msgBox.setText(obs_module_text("OBSWebSocket.Settings.Save.UserPasswordWarningMessage"));
		msgBox.setInformativeText(obs_module_text("OBSWebSocket.Settings.Save.UserPasswordWarningInfoText"));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::No);
		int ret = msgBox.exec();

		switch (ret) {
		case QMessageBox::Yes:
			break;
		case QMessageBox::No:
		default:
			ui->serverPasswordLineEdit->setText(QString::fromStdString(conf->ServerPassword));
			return;
		}
	}

	const bool clientEnabled = ui->enableWebSocketClientCheckBox->isChecked();
	const bool clientAuthRequired = ui->clientAuthRequiredCheckBox->isChecked();
	const ParsedClientConnection parsedClientConnection = ParseClientConnectionInput(ui->clientHostLineEdit->text());

	if (clientEnabled && !parsedClientConnection.valid) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(obs_module_text(parsedClientConnection.errorTitleKey));
		msgBox.setText(obs_module_text(parsedClientConnection.errorMessageKey));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.exec();
		return;
	}

	const bool clientUseTls = parsedClientConnection.valid ? parsedClientConnection.useTls : conf->ClientUseTls.load();
	const std::string clientHost = parsedClientConnection.valid ? parsedClientConnection.host : conf->ClientHost;
	const uint16_t clientPort = parsedClientConnection.valid && parsedClientConnection.port.has_value()
					    ? *parsedClientConnection.port
					    : conf->ClientPort.load();

#if !OBS_WEBSOCKET_CLIENT_TLS
	if (clientEnabled && clientUseTls) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(obs_module_text("OBSWebSocket.Settings.ClientHostInvalidTitle"));
		msgBox.setText(obs_module_text("OBSWebSocket.Settings.ClientTlsDisabledMessage"));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.exec();
		return;
	}
#endif

	if (clientEnabled && !clientUseTls && !ui->clientAllowInsecureCheckBox->isChecked()) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(obs_module_text("OBSWebSocket.Settings.ClientInsecureBlockedTitle"));
		msgBox.setText(obs_module_text("OBSWebSocket.Settings.ClientInsecureBlockedMessage"));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.exec();
		return;
	}

	if (clientEnabled && clientAuthRequired && ui->clientPasswordLineEdit->text().length() < 6) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(obs_module_text("OBSWebSocket.Settings.Save.PasswordInvalidErrorTitle"));
		msgBox.setText(obs_module_text("OBSWebSocket.Settings.Save.PasswordInvalidErrorMessage"));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.exec();
		return;
	}

	if (clientEnabled && clientAuthRequired && clientPasswordManuallyEdited &&
	    (conf->ClientPassword != ui->clientPasswordLineEdit->text().toStdString())) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(obs_module_text("OBSWebSocket.Settings.Save.ClientPasswordWarningTitle"));
		msgBox.setText(obs_module_text("OBSWebSocket.Settings.Save.ClientPasswordWarningMessage"));
		msgBox.setInformativeText(obs_module_text("OBSWebSocket.Settings.Save.ClientPasswordWarningInfoText"));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::No);
		int ret = msgBox.exec();

		switch (ret) {
		case QMessageBox::Yes:
			break;
		case QMessageBox::No:
		default:
			ui->clientPasswordLineEdit->setText(QString::fromStdString(conf->ClientPassword));
			return;
		}
	}

	if (!conf->ClientEnabled && clientEnabled) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(obs_module_text("OBSWebSocket.Settings.ClientEnableWarningTitle"));
		msgBox.setText(obs_module_text("OBSWebSocket.Settings.ClientEnableWarningMessage"));
		msgBox.setInformativeText(obs_module_text("OBSWebSocket.Settings.ClientEnableWarningInfoText"));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::No);
		int ret = msgBox.exec();
		if (ret != QMessageBox::Yes) {
			RefreshData();
			return;
		}
	}

	const bool insecureChanged = (ui->clientAllowInsecureCheckBox->isChecked() && !conf->ClientAllowInsecure) ||
				     (!clientUseTls && conf->ClientUseTls) ||
				     (ui->clientAllowInvalidCertCheckBox->isChecked() && !conf->ClientAllowInvalidCert);
	if (clientEnabled && insecureChanged) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(obs_module_text("OBSWebSocket.Settings.ClientInsecureWarningTitle"));
		msgBox.setText(obs_module_text("OBSWebSocket.Settings.ClientInsecureWarningMessage"));
		msgBox.setInformativeText(obs_module_text("OBSWebSocket.Settings.ClientInsecureWarningInfoText"));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::No);
		int ret = msgBox.exec();
		if (ret != QMessageBox::Yes) {
			RefreshData();
			return;
		}
	}

	bool serverNeedsRestart = (conf->ServerEnabled != ui->enableWebSocketServerCheckBox->isChecked()) ||
				  (conf->ServerPort != ui->serverPortSpinBox->value()) ||
				  (ui->enableAuthenticationCheckBox->isChecked() &&
				   conf->ServerPassword != ui->serverPasswordLineEdit->text().toStdString());

	bool clientNeedsRestart = (conf->ClientEnabled != clientEnabled) || (conf->ClientHost != clientHost) ||
				  (conf->ClientPort != clientPort) || (conf->ClientUseTls != clientUseTls) ||
				  (conf->ClientAllowInsecure != ui->clientAllowInsecureCheckBox->isChecked()) ||
				  (conf->ClientAllowInvalidCert != ui->clientAllowInvalidCertCheckBox->isChecked()) ||
				  (conf->ClientAuthRequired != clientAuthRequired) ||
				  (clientAuthRequired && conf->ClientPassword != ui->clientPasswordLineEdit->text().toStdString());

	conf->ServerEnabled = ui->enableWebSocketServerCheckBox->isChecked();
	conf->AlertsEnabled = ui->enableSystemTrayAlertsCheckBox->isChecked();
	conf->DebugEnabled = ui->enableDebugLoggingCheckBox->isChecked();
	conf->ServerPort = ui->serverPortSpinBox->value();
	conf->AuthRequired = ui->enableAuthenticationCheckBox->isChecked();
	conf->ServerPassword = ui->serverPasswordLineEdit->text().toStdString();

	conf->ClientEnabled = clientEnabled;
	conf->ClientHost = clientHost;
	conf->ClientPort = clientPort;
	conf->ClientUseTls = clientUseTls;
	conf->ClientAllowInsecure = ui->clientAllowInsecureCheckBox->isChecked();
	conf->ClientAllowInvalidCert = ui->clientAllowInvalidCertCheckBox->isChecked();
	conf->ClientAuthRequired = clientAuthRequired;
	conf->ClientPassword = ui->clientPasswordLineEdit->text().toStdString();

	conf->Save();

	RefreshData();
	connectInfo->RefreshData();

	if (serverNeedsRestart) {
		blog(LOG_INFO, "[SettingsDialog::SaveFormData] A setting was changed which requires a server restart.");
		auto server = GetWebSocketServer();
		server->Stop();
		if (conf->ServerEnabled) {
			server->Start();
		}
	}

	if (clientNeedsRestart) {
		blog(LOG_INFO, "[SettingsDialog::SaveFormData] A setting was changed which requires a client reconnect.");
		auto client = GetWebSocketClient();
		if (client) {
			client->Stop();
			if (conf->ClientEnabled) {
				client->Start();
			}
		}
	}
}

void SettingsDialog::FillSessionTable()
{
	auto webSocketServer = GetWebSocketServer();
	if (!webSocketServer) {
		blog(LOG_ERROR, "[SettingsDialog::FillSessionTable] Unable to fetch websocket server instance!");
		UpdateClientStatus();
		return;
	}

	auto webSocketSessions = webSocketServer->GetWebSocketSessions();
	size_t rowCount = webSocketSessions.size();

	// Manually setting the pixmap size *might* break with highdpi. Not sure though
	QIcon checkIcon = style()->standardIcon(QStyle::SP_DialogOkButton);
	QPixmap checkIconPixmap = checkIcon.pixmap(QSize(25, 25));
	QIcon crossIcon = style()->standardIcon(QStyle::SP_DialogCancelButton);
	QPixmap crossIconPixmap = crossIcon.pixmap(QSize(25, 25));

	// Todo: Make a util for translations so that we don't need to import a bunch of obs libraries in order to use them.
	QString kickButtonText = obs_module_text("OBSWebSocket.SessionTable.KickButtonText");

	ui->websocketSessionTable->setRowCount(rowCount);
	size_t i = 0;
	for (auto session : webSocketSessions) {
		QTableWidgetItem *addressItem = new QTableWidgetItem(QString::fromStdString(session.remoteAddress));
		ui->websocketSessionTable->setItem(i, 0, addressItem);

		uint64_t sessionDuration = QDateTime::currentSecsSinceEpoch() - session.connectedAt;
		QTableWidgetItem *durationItem = new QTableWidgetItem(QTime(0, 0, sessionDuration).toString("hh:mm:ss"));
		ui->websocketSessionTable->setItem(i, 1, durationItem);

		QTableWidgetItem *statsItem =
			new QTableWidgetItem(QString("%1/%2").arg(session.incomingMessages).arg(session.outgoingMessages));
		ui->websocketSessionTable->setItem(i, 2, statsItem);

		QLabel *identifiedLabel = new QLabel();
		identifiedLabel->setAlignment(Qt::AlignCenter);
		if (session.isIdentified) {
			identifiedLabel->setPixmap(checkIconPixmap);
		} else {
			identifiedLabel->setPixmap(crossIconPixmap);
		}
		ui->websocketSessionTable->setCellWidget(i, 3, identifiedLabel);

		QPushButton *invalidateButton = new QPushButton(kickButtonText, this);
		QWidget *invalidateButtonWidget = new QWidget();
		QHBoxLayout *invalidateButtonLayout = new QHBoxLayout(invalidateButtonWidget);
		invalidateButtonLayout->addWidget(invalidateButton);
		invalidateButtonLayout->setAlignment(Qt::AlignCenter);
		invalidateButtonLayout->setContentsMargins(0, 0, 0, 0);
		invalidateButtonWidget->setLayout(invalidateButtonLayout);
		ui->websocketSessionTable->setCellWidget(i, 4, invalidateButtonWidget);
		connect(invalidateButton, &QPushButton::clicked, [=]() { webSocketServer->InvalidateSession(session.hdl); });

		i++;
	}

	UpdateClientStatus();
}

void SettingsDialog::EnableAuthenticationCheckBoxChanged()
{
	if (!ui->enableWebSocketServerCheckBox->isChecked()) {
		ui->serverPasswordLineEdit->setEnabled(false);
		ui->generatePasswordButton->setEnabled(false);
		return;
	}

	if (ui->enableAuthenticationCheckBox->isChecked()) {
		ui->serverPasswordLineEdit->setEnabled(true);
		ui->generatePasswordButton->setEnabled(true);
	} else {
		ui->serverPasswordLineEdit->setEnabled(false);
		ui->generatePasswordButton->setEnabled(false);
	}
}

void SettingsDialog::UpdateServerUiState()
{
	auto conf = GetConfig();
	bool serverEnabled = ui->enableWebSocketServerCheckBox->isChecked();

	ui->serverSettingsGroupBox->setEnabled(serverEnabled);
	if (!serverEnabled) {
		ui->serverPasswordLineEdit->setEnabled(false);
		ui->generatePasswordButton->setEnabled(false);
		return;
	}

	if (conf && conf->PortOverridden)
		ui->serverPortSpinBox->setEnabled(false);

	if (conf && conf->PasswordOverridden) {
		ui->enableAuthenticationCheckBox->setEnabled(false);
		ui->serverPasswordLineEdit->setEnabled(false);
		ui->generatePasswordButton->setEnabled(false);
	} else {
		ui->enableAuthenticationCheckBox->setEnabled(true);
		EnableAuthenticationCheckBoxChanged();
	}
}

void SettingsDialog::UpdateClientUiState()
{
	QSignalBlocker blockClientAllowInvalid(ui->clientAllowInvalidCertCheckBox);
	QSignalBlocker blockClientAllowInsecure(ui->clientAllowInsecureCheckBox);

	bool clientEnabled = ui->enableWebSocketClientCheckBox->isChecked();
	bool authRequired = ui->clientAuthRequiredCheckBox->isChecked();

	ui->clientSettingsGroupBox->setEnabled(clientEnabled);

	ui->clientHostLineEdit->setEnabled(clientEnabled);
	ui->clientAllowInsecureCheckBox->setEnabled(clientEnabled);
	ui->clientAuthRequiredCheckBox->setEnabled(clientEnabled);

#if !OBS_WEBSOCKET_CLIENT_TLS
	ui->clientAllowInvalidCertCheckBox->setChecked(false);
	ui->clientAllowInvalidCertCheckBox->setEnabled(false);
	ui->clientAllowInsecureCheckBox->setChecked(true);
	ui->clientAllowInsecureCheckBox->setEnabled(false);
#else
	bool connectionUsesTls = ui->clientHostLineEdit->text().trimmed().startsWith("wss://", Qt::CaseInsensitive);
	if (!connectionUsesTls)
		ui->clientAllowInvalidCertCheckBox->setChecked(false);
	ui->clientAllowInvalidCertCheckBox->setEnabled(clientEnabled && connectionUsesTls);
#endif

	ui->clientPasswordLineEdit->setEnabled(clientEnabled && authRequired);
	ui->showClientPasswordButton->setEnabled(clientEnabled && authRequired);
	ui->generateClientPasswordButton->setEnabled(clientEnabled && authRequired);
}

void SettingsDialog::UpdateClientStatus()
{
	auto client = GetWebSocketClient();
	if (!client) {
		ui->clientStatusValueLabel->setText(obs_module_text("OBSWebSocket.Settings.ClientStatusUnavailable"));
		return;
	}

	auto status = client->GetStatus();
	QString statusText;
	switch (status.state) {
	case WebSocketClient::State::Disabled:
		statusText = obs_module_text("OBSWebSocket.Settings.ClientStatusDisabled");
		break;
	case WebSocketClient::State::Connecting:
		statusText = obs_module_text("OBSWebSocket.Settings.ClientStatusConnecting");
		break;
	case WebSocketClient::State::Connected:
		statusText = obs_module_text("OBSWebSocket.Settings.ClientStatusConnected");
		break;
	case WebSocketClient::State::Disconnected:
		statusText = obs_module_text("OBSWebSocket.Settings.ClientStatusDisconnected");
		break;
	case WebSocketClient::State::Error:
		statusText = obs_module_text("OBSWebSocket.Settings.ClientStatusError");
		break;
	}

	if (!status.endpoint.empty())
		statusText += QString(" (%1)").arg(QString::fromStdString(status.endpoint));
	if (!status.lastError.empty())
		statusText += QString(" - %1").arg(QString::fromStdString(status.lastError));

	ui->clientStatusValueLabel->setText(statusText);
}

void SettingsDialog::GeneratePasswordButtonClicked()
{
	QString newPassword = QString::fromStdString(Utils::Crypto::GeneratePassword());
	ui->serverPasswordLineEdit->setText(newPassword);
	ui->serverPasswordLineEdit->selectAll();
	passwordManuallyEdited = false;
}

void SettingsDialog::GenerateClientPasswordButtonClicked()
{
	QString newPassword = QString::fromStdString(Utils::Crypto::GeneratePassword());
	ui->clientPasswordLineEdit->setText(newPassword);
	ui->clientPasswordLineEdit->selectAll();
	clientPasswordManuallyEdited = false;
}

void SettingsDialog::ToggleClientPasswordVisibility()
{
	bool showing = ui->clientPasswordLineEdit->echoMode() != QLineEdit::Password;
	SetClientPasswordVisible(ui, !showing);
}

void SettingsDialog::ShowConnectInfoButtonClicked()
{
	if (obs_video_active()) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(obs_module_text("OBSWebSocket.Settings.ShowConnectInfoWarningTitle"));
		msgBox.setText(obs_module_text("OBSWebSocket.Settings.ShowConnectInfoWarningMessage"));
		msgBox.setInformativeText(obs_module_text("OBSWebSocket.Settings.ShowConnectInfoWarningInfoText"));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::No);
		int ret = msgBox.exec();

		switch (ret) {
		case QMessageBox::Yes:
			break;
		case QMessageBox::No:
		default:
			return;
		}
	}

	connectInfo->show();
	connectInfo->activateWindow();
	connectInfo->raise();
	connectInfo->setFocus();
}

void SettingsDialog::PasswordEdited()
{
	passwordManuallyEdited = true;
}

void SettingsDialog::ClientPasswordEdited()
{
	clientPasswordManuallyEdited = true;
}
