#include "settings-dialog.h"

#include <obs-frontend-api.h>
#include <obs-module.h>
#include <QtWidgets/QMessageBox>

#include "../obs-websocket.h"


#define CHANGE_ME "changeme"

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
