#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();
	void showEvent(QShowEvent *event);
	void ToggleShowHide();

private Q_SLOTS:
	void AuthCheckboxChanged();
	void FormAccepted();

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
