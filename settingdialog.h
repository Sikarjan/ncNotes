#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog();
    void showEvent(QShowEvent *e);

    QFont cFont;

private slots:
    void on_buttonBox_accepted();
    void currentFont(QFont font);
    void updateText();

private:
    Ui::SettingDialog *ui;

signals:
    void newFont(QFont font);
};

#endif // SETTINGDIALOG_H
