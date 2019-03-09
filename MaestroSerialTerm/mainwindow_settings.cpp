#include "mainwindow.h"
#include<QSettings>

void MainWindow::saveSettings()
{
    QSettings mySettings;

    mAppDirPath += "\\Settings.ini";
    QSettings(mAppDirPath, QSettings::IniFormat);
    mySettings.setIniCodec("UTF-8");
    mySettings.beginGroup("Serial");

    mySettings.setValue("Baud",ui->comboBox_bauds->currentText());

    qDebug() << mAppDirPath;
    qDebug() << mySettings.value("Baud");

    }

void MainWindow::readSettings()
{
    QSettings mySettings;

    mAppDirPath += "\\Settings.ini";
    QSettings(mAppDirPath, QSettings::IniFormat);
    mySettings.setIniCodec("UTF-8");
    mySettings.beginGroup("Serial");
    QString sBaud=mySettings.value("Baud").toString();

    //Appy setting to UI
    int index = ui->comboBox_bauds->findText(sBaud);
    if( index>=0 )
        ui->comboBox_bauds->setCurrentIndex(index);

    qDebug() << ui->comboBox_bauds->currentIndex() << ui->comboBox_bauds->currentText();
    }
