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

#include "obs-websocket.h"
#include "Config.h"
#include "WSServer.h"
#include "settings-dialog.h"
#include "ui_settings-dialog.h"

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


    AuthCheckboxChanged();
}

void SettingsDialog::showEvent(QShowEvent* event)
{
    Config* conf = Config::Current();

    ui->serverEnabled->setChecked(conf->ServerEnabled);
    ui->serverPort->setValue(conf->ServerPort);
    
    ui->debugEnabled->setChecked(conf->DebugEnabled);

    ui->authRequired->setChecked(conf->AuthRequired);
    ui->password->setText(CHANGE_ME);
}

void SettingsDialog::ToggleShowHide()
{
    if (!isVisible())
        setVisible(true);
    else
        setVisible(false);
}

void SettingsDialog::AuthCheckboxChanged()
{
    if (ui->authRequired->isChecked())
        ui->password->setEnabled(true);
    else
        ui->password->setEnabled(false);
}

void SettingsDialog::FormAccepted()
{
    Config* conf = Config::Current();

    conf->ServerEnabled = ui->serverEnabled->isChecked();
    conf->ServerPort = ui->serverPort->value();
    
    conf->DebugEnabled = ui->debugEnabled->isChecked();

    if (ui->authRequired->isChecked())
    {
        if (ui->password->text() != CHANGE_ME)
        {
            QByteArray pwd = ui->password->text().toUtf8();
            const char *new_password = pwd;

            conf->SetPassword(new_password);
        }

        if (strcmp(Config::Current()->Secret, "") != 0)
            conf->AuthRequired = true;
        else
            conf->AuthRequired = false;
    }
    else
    {
        conf->AuthRequired = false;
    }

    conf->Save();

    if (conf->ServerEnabled)
        WSServer::Instance->Start(conf->ServerPort);
    else
        WSServer::Instance->Stop();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
