#pragma once

#include <QtWidgets/QDialog>
#include <QTimer>

#include "ConnectInfo.h"
#include "ui_SettingsDialog.h"

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget* parent = 0);
	~SettingsDialog();
	void showEvent(QShowEvent *event);
	void closeEvent(QCloseEvent *event);
	void ToggleShowHide();

private Q_SLOTS:
	void DialogButtonClicked(QAbstractButton *button);
	void SaveFormData();
	void FillSessionTable();
	void EnableAuthenticationCheckBoxChanged();
	void GeneratePasswordButtonClicked();
	void ShowConnectInfoButtonClicked();

private:
	Ui::SettingsDialog *ui;
	ConnectInfo *connectInfo;
	QTimer *sessionTableTimer;
};
