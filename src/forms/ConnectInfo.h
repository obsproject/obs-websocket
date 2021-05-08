#pragma once

#include <QtWidgets/QDialog>

#include "ui_ConnectInfo.h"

class ConnectInfo : public QDialog
{
	Q_OBJECT

public:
	explicit ConnectInfo(QWidget* parent = 0);
	~ConnectInfo();
	void showEvent(QShowEvent *event);

private Q_SLOTS:
	void CopyServerIpButtonClicked();
	void CopyServerPortButtonClicked();
	void CopyServerPasswordButtonClicked();

private:
	void DrawQr(QString qrText);
	void SetClipboardText(QString text);

	Ui::ConnectInfo *ui;
};
