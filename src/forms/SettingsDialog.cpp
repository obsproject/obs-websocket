#include <obs-frontend-api.h>
#include <QtWidgets/QMessageBox>

#include "SettingsDialog.h"
#include "../obs-websocket.h"
#include "../Config.h"

SettingsDialog::SettingsDialog(QWidget* parent) :
	QDialog(parent, Qt::Dialog),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);

	connect(ui->buttonBox, &QDialogButtonBox::accepted,
		this, &SettingsDialog::FormAccepted);
}

void SettingsDialog::showEvent(QShowEvent* event) {
	auto conf = GetConfig();
	if (!conf) {
		blog(LOG_INFO, "Unable to retreive config!");
		return;
	}

	ui->enableWebSocketServerCheckBox->setChecked(conf->ServerEnabled);
	ui->enableSystemTrayAlertsCheckBox->setChecked(conf->AlertsEnabled);
	ui->enableDebugLoggingCheckBox->setChecked(conf->DebugEnabled);
	ui->enableAuthenticationCheckBox->setChecked(conf->AuthRequired);
	ui->serverPasswordLineEdit->setText(conf->ServerPassword);
	ui->serverPasswordLineEdit->setEnabled(conf->AuthRequired);
}

void SettingsDialog::ToggleShowHide() {
	if (!isVisible())
		setVisible(true);
	else
		setVisible(false);
}

void SettingsDialog::FormAccepted() {
	auto conf = GetConfig();
	if (!conf) {
		blog(LOG_INFO, "Unable to retreive config!");
		return;
	}

	conf->ServerEnabled = ui->enableWebSocketServerCheckBox->isChecked();
	conf->AlertsEnabled = ui->enableSystemTrayAlertsCheckBox->isChecked();
	conf->DebugEnabled = ui->enableDebugLoggingCheckBox->isChecked();
	conf->AuthRequired = ui->enableAuthenticationCheckBox->isChecked();
	conf->ServerPassword = ui->serverPasswordLineEdit->text();

	conf->Save();
}

SettingsDialog::~SettingsDialog() {
	delete ui;
}