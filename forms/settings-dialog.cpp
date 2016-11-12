#include <obs-frontend-api.h>
#include "settings-dialog.h"
#include "ui_settings-dialog.h"
#include "Config.h"

#define CHANGE_ME "changeme"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
	
	connect(ui->authRequired, &QCheckBox::stateChanged, this, &SettingsDialog::AuthCheckboxChanged);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::FormAccepted);

	AuthCheckboxChanged();
}

void SettingsDialog::showEvent(QShowEvent *event) {
	ui->authRequired->setChecked(Config::Current()->AuthRequired);
	ui->password->setText(CHANGE_ME);
}

void SettingsDialog::ToggleShowHide() {
	if (!isVisible()) {
		setVisible(true);
	}
	else {
		setVisible(false);
	}
}

void SettingsDialog::AuthCheckboxChanged() {
	if (ui->authRequired->isChecked()) {
		ui->password->setEnabled(true);
	}
	else {
		ui->password->setEnabled(false);
	}
}

void SettingsDialog::FormAccepted() {
	if (ui->authRequired->isChecked()) {
		if (ui->password->text() != CHANGE_ME) {
			QByteArray pwd = ui->password->text().toLocal8Bit();
			const char *new_password = pwd;

			blog(LOG_INFO, "new password : %s", new_password);
			Config::Current()->SetPassword(new_password);
		}
		
		if (strcmp(Config::Current()->Challenge, "") != 0) {
			Config::Current()->AuthRequired = true;
		}
		else {
			Config::Current()->AuthRequired = false;
		}
	}
	else {
		Config::Current()->AuthRequired = false;
	}
	
	obs_frontend_save();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
