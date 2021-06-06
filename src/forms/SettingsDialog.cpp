#include <obs-module.h>
#include <obs-frontend-api.h>
#include <QtWidgets/QMessageBox>
#include <QDateTime>
#include <QTime>

#include "SettingsDialog.h"
#include "../obs-websocket.h"
#include "../Config.h"
#include "../WebSocketServer.h"

#include "../plugin-macros.generated.h"

SettingsDialog::SettingsDialog(QWidget* parent) :
	QDialog(parent, Qt::Dialog),
	ui(new Ui::SettingsDialog),
	connectInfo(new ConnectInfo),
	sessionTableTimer(new QTimer)
{
	ui->setupUi(this);
	ui->websocketSessionTable->horizontalHeader()->resizeSection(3, 100);
	ui->websocketSessionTable->horizontalHeader()->resizeSection(4, 100);

	connect(sessionTableTimer, &QTimer::timeout,
		this, &SettingsDialog::FillSessionTable);
	connect(ui->buttonBox, &QDialogButtonBox::clicked,
		this, &SettingsDialog::DialogButtonClicked);
	connect(ui->enableAuthenticationCheckBox, &QCheckBox::stateChanged,
		this, &SettingsDialog::EnableAuthenticationCheckBoxChanged);
	connect(ui->generatePasswordButton, &QPushButton::clicked,
		this, &SettingsDialog::GeneratePasswordButtonClicked);
	connect(ui->showConnectInfoButton, &QPushButton::clicked,
		this, &SettingsDialog::ShowConnectInfoButtonClicked);
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
	delete connectInfo;
	delete sessionTableTimer;
}

void SettingsDialog::showEvent(QShowEvent *event)
{
	auto conf = GetConfig();
	if (!conf) {
		blog(LOG_ERROR, "[SettingsDialog::showEvent] Unable to retreive config!");
		return;
	}

	ui->enableWebSocketServerCheckBox->setChecked(conf->ServerEnabled);
	ui->enableSystemTrayAlertsCheckBox->setChecked(conf->AlertsEnabled);
	ui->enableDebugLoggingCheckBox->setChecked(conf->DebugEnabled);
	ui->enableAuthenticationCheckBox->setChecked(conf->AuthRequired);
	ui->serverPasswordLineEdit->setText(conf->ServerPassword);
	ui->serverPasswordLineEdit->setEnabled(conf->AuthRequired);
	ui->serverPortSpinBox->setValue(conf->ServerPort);

	if (conf->PortOverridden) {
		ui->serverPortSpinBox->setEnabled(false);
	}

	if (conf->PasswordOverridden) {
		ui->enableAuthenticationCheckBox->setEnabled(false);
		ui->serverPasswordLineEdit->setEnabled(false);
	}

	FillSessionTable();

	sessionTableTimer->start(1000);
}

void SettingsDialog::closeEvent(QCloseEvent *event)
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
	connectInfo->hide();

	auto conf = GetConfig();
	if (!conf) {
		blog(LOG_ERROR, "[SettingsDialog::SaveFormData] Unable to retreive config!");
		return;
	}

	bool needsRestart = false;

	// I decided not to restart the server if debug is changed. Might mess with peoples' scripts
	if (conf->ServerEnabled != ui->enableWebSocketServerCheckBox->isChecked()) {
		needsRestart = true;
	} else if (conf->AuthRequired != ui->enableAuthenticationCheckBox->isChecked()) {
		needsRestart = true;
	} else if (conf->ServerPassword != ui->serverPasswordLineEdit->text()) {
		needsRestart = true;
	} else if (conf->ServerPort != ui->serverPortSpinBox->value()) {
		needsRestart = true;
	}

	conf->ServerEnabled = ui->enableWebSocketServerCheckBox->isChecked();
	conf->AlertsEnabled = ui->enableSystemTrayAlertsCheckBox->isChecked();
	conf->DebugEnabled = ui->enableDebugLoggingCheckBox->isChecked();
	conf->AuthRequired = ui->enableAuthenticationCheckBox->isChecked();
	conf->ServerPassword = ui->serverPasswordLineEdit->text();
	conf->ServerPort = ui->serverPortSpinBox->value();

	conf->Save();

	if (needsRestart) {
		auto server = GetWebSocketServer();
		server->Stop();
		if (conf->ServerEnabled) {
			server->Start();
		}
	}
}

void SettingsDialog::FillSessionTable()
{
	auto webSocketServer = GetWebSocketServer();
	if (!webSocketServer) {
		blog(LOG_ERROR, "[SettingsDialog::FillSessionTable] Unable to fetch websocket server instance!");
		return;
	}

	auto webSocketSessions = webSocketServer->GetWebSocketSessions();
	size_t rowCount = webSocketSessions.size();

	// Manually setting the pixmap size *might* break with highdpi. Not sure though
	QIcon checkIcon = style()->standardIcon(QStyle::SP_DialogOkButton);
	QPixmap checkIconPixmap = checkIcon.pixmap(QSize(25, 25));
	QIcon crossIcon = style()->standardIcon(QStyle::SP_DialogCancelButton);
	QPixmap crossIconPixmap = crossIcon.pixmap(QSize(25, 25));

	obs_frontend_push_ui_translation(obs_module_get_string);
	QString kickButtonText = QObject::tr("OBSWebSocket.SessionTable.KickButtonText");
	obs_frontend_pop_ui_translation();

	ui->websocketSessionTable->setRowCount(rowCount);
	size_t i = 0;
	for (auto session : webSocketSessions) {
		QTableWidgetItem *addressItem = new QTableWidgetItem(QString::fromStdString(session.remoteAddress));
		ui->websocketSessionTable->setItem(i, 0, addressItem);

		uint64_t sessionDuration = QDateTime::currentSecsSinceEpoch() - session.connectedAt;
		QTableWidgetItem *durationItem = new QTableWidgetItem(QTime(0, 0, sessionDuration).toString("hh:mm:ss"));
		ui->websocketSessionTable->setItem(i, 1, durationItem);

		QTableWidgetItem *statsItem = new QTableWidgetItem(QString("%1/%2").arg(session.incomingMessages).arg(session.outgoingMessages));
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
		connect(invalidateButton, &QPushButton::clicked, [=]() {
			webSocketServer->InvalidateSession(session.hdl);
		});

		i++;
	}
}

void SettingsDialog::EnableAuthenticationCheckBoxChanged()
{
	if (ui->enableAuthenticationCheckBox->isChecked()) {
		ui->serverPasswordLineEdit->setEnabled(true);
		ui->generatePasswordButton->setEnabled(true);
	} else {
		ui->serverPasswordLineEdit->setEnabled(false);
		ui->generatePasswordButton->setEnabled(false);
	}
}

void SettingsDialog::GeneratePasswordButtonClicked()
{
	QString newPassword = Utils::Crypto::GeneratePassword();
	ui->serverPasswordLineEdit->setText(newPassword);
	ui->serverPasswordLineEdit->selectAll();
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
