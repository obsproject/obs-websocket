#include <obs-frontend-api.h>
#include <QtWidgets/QMessageBox>

#include "SettingsDialog.h"
#include "../obs-websocket.h"

SettingsDialog::SettingsDialog(QWidget* parent) :
	QDialog(parent, Qt::Dialog),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);

	connect(ui->buttonBox, &QDialogButtonBox::accepted,
		this, &SettingsDialog::FormAccepted);
}

void SettingsDialog::showEvent(QShowEvent* event) {
	;
}

void SettingsDialog::ToggleShowHide() {
	if (!isVisible())
		setVisible(true);
	else
		setVisible(false);
}

void SettingsDialog::FormAccepted() {
	;
}

SettingsDialog::~SettingsDialog() {
	delete ui;
}