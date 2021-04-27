#pragma once

#include <QtWidgets/QDialog>

#include "ui_SettingsDialog.h"

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget* parent = 0);
	~SettingsDialog();
	void showEvent(QShowEvent* event);
	void ToggleShowHide();

private Q_SLOTS:
	void FormAccepted();
	void EnableAuthenticationCheckBoxChanged();
	void CopyPasswordButtonClicked();

private:
	Ui::SettingsDialog* ui;

	void FillSessionTable();
};