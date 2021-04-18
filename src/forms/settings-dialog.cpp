/*
obs-websocket
Copyright (C) 2016-2017	Stéphane Lepin <stephane.lepin@gmail.com>

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

#include "settings-dialog.h"

#include <obs-frontend-api.h>
#include <obs-module.h>
#include <QtWidgets/QMessageBox>

#include "../obs-websocket.h"
#include "../Config.h"
#include "../WSServer.h"


#define CHANGE_ME "changeme"

SettingsDialog::SettingsDialog(QWidget* parent) :
	QDialog(parent, Qt::Dialog),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);

	connect(ui->authRequired, &QCheckBox::stateChanged,
		this, &SettingsDialog::AuthCheckboxChanged);
	connect(ui->buttonBox, &QDialogButtonBox::accepted,
		this, &SettingsDialog::FormAccepted);
}

void SettingsDialog::showEvent(QShowEvent* event) {
	auto conf = GetConfig();
	if (conf) {
		ui->serverEnabled->setChecked(conf->ServerEnabled);
		ui->serverPort->setValue(conf->ServerPort);
		ui->lockToIPv4->setChecked(conf->LockToIPv4);

		ui->debugEnabled->setChecked(conf->DebugEnabled);
		ui->alertsEnabled->setChecked(conf->AlertsEnabled);

		ui->authRequired->blockSignals(true);
		ui->authRequired->setChecked(conf->AuthRequired);
		ui->authRequired->blockSignals(false);
	}

	ui->password->setText(CHANGE_ME);
	ui->password->setEnabled(ui->authRequired->isChecked());
}

void SettingsDialog::ToggleShowHide() {
	if (!isVisible())
		setVisible(true);
	else
		setVisible(false);
}

void SettingsDialog::PreparePasswordEntry() {
	ui->authRequired->blockSignals(true);
	ui->authRequired->setChecked(true);
	ui->authRequired->blockSignals(false);
	ui->password->setEnabled(true);
	ui->password->setFocus();
}

void SettingsDialog::AuthCheckboxChanged() {
	if (ui->authRequired->isChecked()) {
		ui->password->setEnabled(true);
	}
	else {
		obs_frontend_push_ui_translation(obs_module_get_string);
		QString authDisabledWarning = QObject::tr("OBSWebsocket.Settings.AuthDisabledWarning");
		obs_frontend_pop_ui_translation();

		QMessageBox::StandardButton response = QMessageBox::question(this, "obs-websocket", authDisabledWarning);
		if (response == QMessageBox::Yes) {
			ui->password->setEnabled(false);
		} else {
			ui->authRequired->setChecked(true);
		}
	}
}

void SettingsDialog::FormAccepted() {
	auto conf = GetConfig();
	if (!conf) {
		return;
	}

	conf->ServerEnabled = ui->serverEnabled->isChecked();
	conf->ServerPort = ui->serverPort->value();
	conf->LockToIPv4 = ui->lockToIPv4->isChecked();

	conf->DebugEnabled = ui->debugEnabled->isChecked();
	conf->AlertsEnabled = ui->alertsEnabled->isChecked();

	if (ui->authRequired->isChecked()) {
		if (ui->password->text() != CHANGE_ME) {
			conf->SetPassword(ui->password->text());
		}

		if (!conf->Secret.isEmpty())
			conf->AuthRequired = true;
		else
			conf->AuthRequired = false;
	}
	else
	{
		conf->AuthRequired = false;
	}

	conf->Save();

	auto server = GetServer();
	if (conf->ServerEnabled) {
		server->start(conf->ServerPort, conf->LockToIPv4);
	} else {
		server->stop();
	}
}

SettingsDialog::~SettingsDialog() {
	delete ui;
}
