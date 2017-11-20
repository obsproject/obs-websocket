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
#include "../WSServer.h"
#include "settings-dialog.h"
#include "ui_settings-dialog.h"
#include "../Utils.h"

#define CHANGE_ME "changeme"

SettingsDialog::SettingsDialog(QWidget* parent) :
    QDialog(parent, Qt::Dialog),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    connect(ui->wampEnabled, &QCheckBox::stateChanged,
            this, &SettingsDialog::WampCheckboxChanged);
    connect(ui->wampIdEnabled, &QCheckBox::stateChanged,
        this, &SettingsDialog::WampIdCheckboxChanged);
    connect(ui->wampAuthEnabled, &QCheckBox::stateChanged,
            this, &SettingsDialog::WampAuthCheckboxChanged);
    connect(ui->wampUrl, &QLineEdit::textChanged,
            this, &SettingsDialog::WampUrlChanged);
    connect(ui->wampId, &QLineEdit::textChanged,
            this, &SettingsDialog::WampIdChanged);
    connect(ui->wampBaseUri, &QLineEdit::textChanged,
            this, &SettingsDialog::WampBaseUriChanged);
    connect(ui->authRequired, &QCheckBox::stateChanged,
            this, &SettingsDialog::AuthCheckboxChanged);
    connect(ui->buttonBox, &QDialogButtonBox::accepted,
        this, &SettingsDialog::FormAccepted);


    AuthCheckboxChanged();
}

void SettingsDialog::showEvent(QShowEvent* event) {
    Q_UNUSED(event);
    Config* conf = Config::Current();

    ui->serverEnabled->setChecked(conf->ServerEnabled);
    ui->serverPort->setValue(conf->ServerPort);

    ui->debugEnabled->setChecked(conf->DebugEnabled);
    ui->alertsEnabled->setChecked(conf->AlertsEnabled);

    ui->authRequired->setChecked(conf->AuthRequired);
    ui->password->setText(CHANGE_ME);
    
    ui->wampEnabled->setChecked(conf->WampEnabled);
    ui->wampIdEnabled->setChecked(conf->WampIdEnabled);
    ui->wampAuthEnabled->setChecked(conf->WampAuthEnabled);
    ui->wampUrl->setText(conf->WampUrl.toString());
    ui->wampId->setText(conf->WampId);
    ui->wampBaseUri->setText(conf->WampBaseUri);
    ui->wampRealm->setText(conf->WampRealm);
    ui->wampUser->setText(conf->WampUser);
    ui->wampPassword->setText(CHANGE_ME);
    ui->wampAnonFB->setChecked(conf->WampAnonymousFallback);
    ui->wampRegProc->setText(conf->WampRegisterProcedure);
}

void SettingsDialog::ToggleShowHide() {
    if (!isVisible())
        setVisible(true);
    else
        setVisible(false);
}

void SettingsDialog::AuthCheckboxChanged() {
    if (ui->authRequired->isChecked())
        ui->password->setEnabled(true);
    else
        ui->password->setEnabled(false);
}

void SettingsDialog::FormAccepted() {
    Config* conf = Config::Current();

    conf->ServerEnabled = ui->serverEnabled->isChecked();
    conf->ServerPort = ui->serverPort->value();

    conf->DebugEnabled = ui->debugEnabled->isChecked();
    conf->AlertsEnabled = ui->alertsEnabled->isChecked();

    if (ui->authRequired->isChecked()) {
        if (ui->password->text() != CHANGE_ME) {
            conf->SetPassword(ui->password->text());
        }

        if (!Config::Current()->Secret.isEmpty())
            conf->AuthRequired = true;
        else
            conf->AuthRequired = false;
    }
    else
    {
        conf->AuthRequired = false;
    }

    conf->WampEnabled = ui->wampEnabled->isChecked();
    conf->WampAlertsEnabled = ui->wampAlertsEnabled->isChecked();

    conf->WampUrl=QUrl(ui->wampUrl->text());
    conf->WampRealm=ui->wampRealm->text();

    conf->WampIdEnabled = ui->wampIdEnabled->isChecked();
    conf->WampAuthEnabled = ui->wampAuthEnabled->isChecked();

    if (conf->WampAuthEnabled) {
        conf->WampUser=ui->wampUser->text();
        if (ui->wampPassword->text() != CHANGE_ME)
            conf->WampPassword=ui->wampPassword->text();
    }
    
    conf->WampBaseUri = ui->wampBaseUri->text();
    if (conf->WampIdEnabled && !ui->wampId->text().isEmpty())
    {
        conf->WampId=ui->wampId->text();
        conf->WampRegisterProcedure=ui->wampRegProc->text();
    }

    conf->Save();

    if (conf->ServerEnabled)
        WSServer::Instance->Start(conf->ServerPort);
    else
        WSServer::Instance->Stop();

    if (conf->WampEnabled)
        WSServer::Instance->StartWamp(conf->WampUrl, conf->WampRealm, conf->WampIdEnabled?conf->WampId:QString(), conf->WampAuthEnabled?conf->WampUser:QString(), conf->WampAuthEnabled?conf->WampPassword:QString());
    else
        WSServer::Instance->StopWamp();
}

void SettingsDialog::WampIdChanged(QString wampId)
{
    ui->wampId->setText(Utils::WampUrlFix(wampId));
}

void SettingsDialog::WampBaseUriChanged(QString wampUri)
{
    ui->wampBaseUri->setText(Utils::WampUrlFix(wampUri));
}

void SettingsDialog::WampUrlChanged(QString newUrl)
{
    if (newUrl.isEmpty())
        newUrl = ui->wampUrl->text();
    
    QPushButton * okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    
    if (!ui->wampEnabled)
    {
        okButton->setEnabled(true);
        return;
    }
    
    if (newUrl.startsWith(QStringLiteral("ws://")) || newUrl.startsWith(QStringLiteral("wss://")))
        okButton->setEnabled(QUrl(newUrl).isValid());
    else
        okButton->setEnabled(false);
    
    
    if (okButton->isEnabled())
    {
        ui->wampUrl->setStyleSheet("color: black;");
    }
    else
    {
        ui->wampUrl->setStyleSheet("color: red; text-decoration: underline;");
    }
}

void SettingsDialog::WampCheckboxChanged()
{
    if (ui->wampEnabled->isChecked())
    {
        ui->wampUrl->setEnabled(true);
        ui->wampRealm->setEnabled(true);
        ui->wampIdEnabled->setEnabled(true);
        ui->wampId->setEnabled(ui->wampIdEnabled->isChecked());
        ui->wampRegProc->setEnabled(ui->wampIdEnabled->isChecked());
        ui->wampAuthEnabled->setEnabled(true);
        ui->wampAnonFB->setEnabled(true);
        ui->wampUser->setEnabled(ui->wampAuthEnabled->isChecked());
        ui->wampPassword->setEnabled(ui->wampAuthEnabled->isChecked());
    }
    else
    {
        ui->wampUrl->setEnabled(false);
        ui->wampRealm->setEnabled(false);
        ui->wampIdEnabled->setEnabled(false);
        ui->wampId->setEnabled(false);
        ui->wampRegProc->setEnabled(false);
        ui->wampAuthEnabled->setEnabled(false);
        ui->wampAnonFB->setEnabled(false);
        ui->wampUser->setEnabled(false);
        ui->wampPassword->setEnabled(false);
    }
    WampUrlChanged(ui->wampUrl->text());
}

void SettingsDialog::WampAuthCheckboxChanged()
{
    ui->wampUser->setEnabled(ui->wampAuthEnabled->isChecked());
    ui->wampPassword->setEnabled(ui->wampAuthEnabled->isChecked());
}

void SettingsDialog::WampIdCheckboxChanged()
{
    ui->wampId->setEnabled(ui->wampIdEnabled->isChecked());
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}
