#include <obs-frontend-api.h>
#include <QtWidgets/QMessageBox>
#include <QClipboard>

#include "SettingsDialog.h"
#include "../obs-websocket.h"
#include "../Config.h"

SettingsDialog::SettingsDialog(QWidget* parent) :
	QDialog(parent, Qt::Dialog),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);
	ui->websocketSessionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	connect(ui->buttonBox, &QDialogButtonBox::accepted,
		this, &SettingsDialog::FormAccepted);
	connect(ui->enableAuthenticationCheckBox, &QCheckBox::stateChanged,
		this, &SettingsDialog::EnableAuthenticationCheckBoxChanged);
	connect(ui->copyPasswordButton, &QPushButton::clicked,
		this, &SettingsDialog::CopyPasswordButtonClicked);
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}

void SettingsDialog::showEvent(QShowEvent* event)
{
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

	FillSessionTable();
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
	int rowCount = 5;
	ui->websocketSessionTable->setRowCount(rowCount);
	for (int i = 0; i < 5; i++) {
		QTableWidgetItem *addressItem = new QTableWidgetItem("test");
		ui->websocketSessionTable->setItem(i, 0, addressItem);

		QTableWidgetItem *durationItem = new QTableWidgetItem("test");
		ui->websocketSessionTable->setItem(i, 1, durationItem);

		QTableWidgetItem *statsItem = new QTableWidgetItem("test");
		ui->websocketSessionTable->setItem(i, 2, statsItem);

		QPushButton *invalidateButton = new QPushButton("Kick", this);
		QWidget *invalidateButtonWidget = new QWidget();
		QHBoxLayout *invalidateButtonLayout = new QHBoxLayout(invalidateButtonWidget);
		invalidateButtonLayout->addWidget(invalidateButton);
		invalidateButtonLayout->setAlignment(Qt::AlignCenter);
		invalidateButtonLayout->setContentsMargins(0, 0, 0, 0);
		invalidateButtonWidget->setLayout(invalidateButtonLayout);
		ui->websocketSessionTable->setCellWidget(i, 3, invalidateButtonWidget);
	}
}

void SettingsDialog::FormAccepted()
{
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