#include "settingdialog.h"
#include "ui_settingdialog.h"

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);
}

SettingDialog::~SettingDialog()
{
    delete ui;
}

void SettingDialog::showEvent(QShowEvent *e)
{
    qDebug() << cFont;
    currentFont(cFont);
}

void SettingDialog::on_buttonBox_accepted()
{
    QFont font = ui->fontType->currentFont();
    font.setPixelSize(ui->fontSize->value());
    font.setPointSize(ui->fontSize->value());
    emit newFont(font);
}

void SettingDialog::currentFont(QFont font)
{
    ui->fontSize->setValue(font.pointSize());
    ui->fontType->setCurrentFont(font);
}

void SettingDialog::updateText()
{
    QFont font = ui->fontType->font();
    font.setPixelSize(ui->fontSize->value());
    ui->testLabel->setFont(font);
}
