#include "dialog.h"
#include "ui_dialog.h"
#include <QtCore/QSettings>
#include "globals.h"

BouyomiConfigDialog::BouyomiConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BouyomiConfigDialog)
{
    ui->setupUi(this);

    QSettings cfg(QString::fromStdString(Globals::getConfigFilePath()), QSettings::IniFormat);
    ui->checkBox->setChecked(cfg.value("sendToMyself", DEFAULTCONFIG_SENDTOMYSELF).toBool());
    ui->cbTextMessageTargetModeClient->setChecked(cfg.value("textMessageTargetModeClient", DEFAULTCONFIG_TEXTMESSAGETARGETMODECLIENT).toBool());
    ui->cbTextMessageTargetModeChannel->setChecked(cfg.value("textMessageTargetModeChannel", DEFAULTCONFIG_TEXTMESSAGETARGETMODECHANNEL).toBool());
    ui->cbTextMessageTargetModeServer->setChecked(cfg.value("textMessageTargetModeServer", DEFAULTCONFIG_TEXTMESSAGETARGETMODESERVER).toBool());
}

BouyomiConfigDialog::~BouyomiConfigDialog()
{
    delete ui;
}

void BouyomiConfigDialog::accept() {

    QSettings cfg(QString::fromStdString(Globals::getConfigFilePath()), QSettings::IniFormat);
    cfg.setValue("sendToMyself", ui->checkBox->checkState());


    cfg.setValue("textMessageTargetModeClient", ui->cbTextMessageTargetModeClient->checkState());
    cfg.setValue("textMessageTargetModeChannel", ui->cbTextMessageTargetModeChannel->checkState());
    cfg.setValue("textMessageTargetModeServer", ui->cbTextMessageTargetModeServer->checkState());


    Globals::sendToMyself = ui->checkBox->checkState();
    Globals::textMessageTargetModeClient = ui->cbTextMessageTargetModeClient->checkState();
    Globals::textMessageTargetModeChannel = ui->cbTextMessageTargetModeChannel->checkState();
    Globals::textMessageTargetModeServer = ui->cbTextMessageTargetModeServer->checkState();
    QDialog::accept();
}

void BouyomiConfigDialog::reject() {
    QDialog::reject();
}
