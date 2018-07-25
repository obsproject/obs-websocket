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

#include <obs-frontend-api.h>

#include "../obs-websocket.h"
#include "../Config.h"
#include "src/WSServer.h"
#include "settings-dialog.h"

#define CHANGE_ME "changeme"

SettingsDialog::SettingsDialog(QWidget* parent) :
	QDialog(parent, Qt::Dialog),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);

	connect(ui->authRequired, &QCheckBox::stateChanged,
			ui->password, &QLineEdit::setEnabled);
	connect(ui->buttonBox, &QDialogButtonBox::accepted,
			this, &SettingsDialog::FormAccepted);
}

void SettingsDialog::showEvent(QShowEvent* event)
{
	QSharedPointer<Config> conf = Config::Current();

	ui->serverEnabled->setChecked(conf->ServerEnabled);
	ui->serverPort->setValue(conf->ServerPort);

	ui->debugEnabled->setChecked(conf->DebugEnabled);
	ui->alertsEnabled->setChecked(conf->AlertsEnabled);

	ui->authRequired->setChecked(conf->AuthRequired);
	ui->password->setText(conf->AuthPassword);

	ui->password->setEnabled(ui->authRequired->isChecked());
}

void SettingsDialog::ToggleShowHide()
{
	setVisible(!isVisible());
}

void SettingsDialog::FormAccepted()
{
	QSharedPointer<Config> conf = Config::Current();

	conf->ServerEnabled = ui->serverEnabled->isChecked();
	conf->ServerPort = (quint16)ui->serverPort->value();

	conf->DebugEnabled = ui->debugEnabled->isChecked();
	conf->AlertsEnabled = ui->alertsEnabled->isChecked();

	conf->AuthRequired = ui->authRequired->isChecked();
	conf->AuthPassword = ui->password->text();

	conf->Save();

	if (conf->ServerEnabled)
		WSServer::Current()->start(conf->ServerPort);
	else
		WSServer::Current()->stop();
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}
