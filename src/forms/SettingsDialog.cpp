#include <obs-module.h>
#include <obs-frontend-api.h>
#include <QtWidgets/QMessageBox>
#include <QClipboard>
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
	sessionTableTimer(new QTimer)
{
	ui->setupUi(this);
	ui->websocketSessionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	connect(ui->buttonBox, &QDialogButtonBox::accepted,
		this, &SettingsDialog::FormAccepted);
	connect(ui->enableAuthenticationCheckBox, &QCheckBox::stateChanged,
		this, &SettingsDialog::EnableAuthenticationCheckBoxChanged);
	connect(ui->copyPasswordButton, &QPushButton::clicked,
		this, &SettingsDialog::CopyPasswordButtonClicked);
	connect(sessionTableTimer, &QTimer::timeout,
		this, &SettingsDialog::FillSessionTable);
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
	delete sessionTableTimer;
}

void SettingsDialog::showEvent(QShowEvent *event)
{
	auto conf = GetConfig();
	if (!conf) {
		blog(LOG_ERROR, "[showEvent] Unable to retreive config!");
		return;
	}

	ui->enableWebSocketServerCheckBox->setChecked(conf->ServerEnabled);
	ui->enableSystemTrayAlertsCheckBox->setChecked(conf->AlertsEnabled);
	ui->enableDebugLoggingCheckBox->setChecked(conf->DebugEnabled);
	ui->enableAuthenticationCheckBox->setChecked(conf->AuthRequired);
	ui->serverPasswordLineEdit->setText(conf->ServerPassword);
	ui->serverPasswordLineEdit->setEnabled(conf->AuthRequired);

	FillSessionTable();

	sessionTableTimer->start(1000);
}

void SettingsDialog::closeEvent(QCloseEvent *event)
{
	if (sessionTableTimer->isActive())
		sessionTableTimer->stop();
}

void SettingsDialog::ToggleShowHide()
{
	if (!isVisible())
		setVisible(true);
	else
		setVisible(false);
}

void SettingsDialog::FillSessionTable()
{
	auto webSocketServer = GetWebSocketServer();
	if (!webSocketServer) {
		blog(LOG_ERROR, "[FillSessionTable] Unable to fetch websocket server instance!");
		return;
	}

	auto webSocketSessions = webSocketServer->GetWebSocketSessions();
	size_t rowCount = webSocketSessions.size();

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

		QPushButton *invalidateButton = new QPushButton(kickButtonText, this);
		QWidget *invalidateButtonWidget = new QWidget();
		QHBoxLayout *invalidateButtonLayout = new QHBoxLayout(invalidateButtonWidget);
		invalidateButtonLayout->addWidget(invalidateButton);
		invalidateButtonLayout->setAlignment(Qt::AlignCenter);
		invalidateButtonLayout->setContentsMargins(0, 0, 0, 0);
		invalidateButtonWidget->setLayout(invalidateButtonLayout);
		ui->websocketSessionTable->setCellWidget(i, 3, invalidateButtonWidget);
		connect(invalidateButton, &QPushButton::clicked, [=]() {
			webSocketServer->InvalidateSession(session.hdl);
		});

		i++;
	}
}

void SettingsDialog::FormAccepted()
{
	auto conf = GetConfig();
	if (!conf) {
		blog(LOG_ERROR, "[FormAccepted] Unable to retreive config!");
		return;
	}

	conf->ServerEnabled = ui->enableWebSocketServerCheckBox->isChecked();
	conf->AlertsEnabled = ui->enableSystemTrayAlertsCheckBox->isChecked();
	conf->DebugEnabled = ui->enableDebugLoggingCheckBox->isChecked();
	conf->AuthRequired = ui->enableAuthenticationCheckBox->isChecked();
	conf->ServerPassword = ui->serverPasswordLineEdit->text();

	conf->Save();
}

void SettingsDialog::EnableAuthenticationCheckBoxChanged()
{
	if (ui->enableAuthenticationCheckBox->isChecked()) {
		ui->serverPasswordLineEdit->setEnabled(true);
		ui->copyPasswordButton->setEnabled(true);
	} else {
		ui->serverPasswordLineEdit->setEnabled(false);
		ui->copyPasswordButton->setEnabled(false);
	}
}

void SettingsDialog::CopyPasswordButtonClicked()
{
	QClipboard *clipboard = QGuiApplication::clipboard();
	clipboard->setText(ui->serverPasswordLineEdit->text());
	ui->serverPasswordLineEdit->selectAll();
}
