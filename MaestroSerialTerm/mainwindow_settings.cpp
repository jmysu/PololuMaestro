#include "mainwindow.h"
#include<QSettings>

void MainWindow::saveSettings()
{
    qDebug() << "\n" << Q_FUNC_INFO;

    QSettings *mySettings;
    QString sSettings = configDir()+"/"+ APP_NAME;
    sSettings +=".ini";
    qDebug() << "INI file:" << sSettings << (QFile(sSettings).exists()?"Exist!":"Not Found!");
    mySettings = new QSettings(sSettings, QSettings::IniFormat);


    mySettings->setIniCodec("UTF-8");
    mySettings->beginGroup("Serial");

    mySettings->setValue("Port",ui->comboBox_SerialPort->currentText());
    qDebug() << mySettings->value("Port");
    mySettings->setValue("Baud",ui->comboBox_bauds->currentText());
    qDebug() << mySettings->value("Baud");
    mySettings->endGroup();


    mySettings->beginGroup("Maestro");
    for (int r=0;r<mMaestroChannels;r++) {
        QString sChannelOpt = QString("ChannelOpt%1").arg(r);

        QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(r, E0_OutputOpt);
        QWidget *widget = item->widget();
        QComboBox *combobox = dynamic_cast<QComboBox*>(widget);
        qDebug() << r << combobox->currentText() << sChannelOpt << slSettingsChannelOpt[r];
        if (combobox->currentText() !=  slSettingsChannelOpt[r])  //setting changed
            mySettings->setValue(sChannelOpt, combobox->currentText());
        //qDebug() << mySettings->value(sChannelOpt).toString();
        }
    mySettings->endGroup();

    mySettings->sync();
    }

void MainWindow::readSettings()
{
    qDebug() << "\n" << Q_FUNC_INFO;

    QSettings *mySettings;
    QString sSettings = configDir()+"/"+ APP_NAME;
    sSettings +=".ini";
    qDebug() << "INI file:" << sSettings << (QFile(sSettings).exists()?"Exist!":"Not Found!");
    mySettings = new QSettings(sSettings, QSettings::IniFormat);

    mySettings->setIniCodec("UTF-8");
    mySettings->beginGroup("Serial");
    sSettingsPort=mySettings->value("Port").toString();
    sSettingsBaud=mySettings->value("Baud").toString();
    mySettings->endGroup();

    mySettings->beginGroup("Maestro");
    slSettingsChannelOpt.clear();
    for (int r=0;r<mMaestroChannels;r++) {
        QString sChannelOpt = QString("ChannelOpt%1").arg(r);
        slSettingsChannelOpt <<  mySettings->value(sChannelOpt).toString();
        }
    qDebug() << slSettingsChannelOpt;
    mySettings->endGroup();
    }
