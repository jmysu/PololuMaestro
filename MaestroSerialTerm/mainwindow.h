#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/**************************************************************************************************/

#include <QMainWindow>
#include <QTextBrowser>
#include <QDialog>
#include <QSize>
#include <QScrollBar>
#include <QKeyEvent>
#include <QDateTime>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSlider>
#include <QSpinBox>
#include <QSignalMapper>
#include <QWidget>
#include <QDebug>
#include <qrandom.h>

#include "ui_mainwindow.h"
#include "constants.h"
#include <QStandardPaths>

/**************************************************************************************************/

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();
        QSerialPort *mSerialPort;

        static const int8_t mMaestroChannels  = 12; //Maestro Channels
        static const int8_t mMaestroSpeedInit = 20; //Maestro Init Speed
        static const int8_t mMaestroAccInit   = 40;//Maestro Init Accelation
        static const int16_t mChannelMin      = 1000; //1000us
        static const int16_t mChannelMax      = 2000; //2000us
        //First 6 Channel min/max
        const int16_t mChannelMaxAdj[mMaestroChannels]={-200,0,0,0, 0,0,0,0, 0,0,0,0};
        const int16_t mChannelMinAdj[mMaestroChannels]={+500,0,0,0, 0,0,0,0, 0,0,0,0};

        QStringList slOutputOptions = {"Off", "Servo", "Input", "Output"};

        //Settings
        void saveSettings();
        void readSettings();
        QString sSettingsPort;
        QString sSettingsBaud;
        QStringList slSettingsChannelOpt;
        void MaestroRestoreSettings();


        QString configDir() {//Windows C:\Users\jimmy\AppData\Roaming\MaestroSerialTerm
            return QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first();
            }

        bool bSerialOpen = false;
        QTimer *timerMaestroSweep;
        QTimer *timerMaestroCrazy;

    private:
        Ui::MainWindow *ui;

        QTimer *timerSerialPorts;
        QStringList slAvailableSerialPorts;
        QStringList slSendHistory;
        int iHistoryIdx;
        bool bTimeStampOn;
        bool bTimeStampMS;
        terminal_modes terminal_mode;

        void SerialPortsChecks_timer_init(void);
        void OpenSerialPort(void);
        void CloseSerialPort(void);
        void SerialSendAscii(void);
        void SerialSendHex(void);
        void PrintReceivedData(QTextBrowser* textBrowserAscii, QTextBrowser *textBrowserHex, terminal_modes mode);
        QString GetActualSystemTime(void);

        //---------------------------------------------------------------------
        QByteArray baLastCmd, baRespose;
        void MaestroGoHome();   
        int16_t MaestroGetError();
        int16_t MaestroGetPositionUs(int8_t ch);
        void MaestroSetTargetUs(int8_t ch, int16_t pos);
        void MaestroSetSpeed(int8_t ch, int16_t speed);
        void MaestroSetAcc(int8_t ch, int16_t acc);
        void MaestroTabInit();
        int16_t MaestroGetChannelMin(int8_t channel);
        int16_t MaestroGetChannelMax(int8_t channel);



    private slots:
        void SerialPortsCheck(void);
        void SerialReceive(void);
        void CBoxBaudsChanged(void);
        void CBoxEOLChanged(void);
        void SerialPortErrorHandler(void);

        bool eventFilter(QObject *target, QEvent *event);

        void ButtonOpenPressed(void);
        void ButtonClosePressed(void);
        void ButtonClearPressed(void);
        void ButtonSendAsciiPressed(void);
        void ButtonSendHexPressed(void);
        void MenuBarExitClick(void);
        void MenuBarTermAsciiClick(void);
        void MenuBarTermHexClick(void);
        void MenuBarTermBothAsciiHexClick(void);
        void MenuBarTimestampClick(void);
        void MenuBarTimestampMsClick(void);
        void MenuBarAboutClick(void);

        //---------------------------------------------------------------------
        void slotChannelOptChanged();
        void slotTargetSpinBox();
        void slotTargetSlider();
        void slotSpeedSpinBox();
        void slotAccSpinBox();
        void slotUpdatePosition(int8_t ch);

        void on_pushButtonReadError_clicked();
        void on_pushButtonInit_clicked();
        void on_spinBoxSweep_editingFinished();
        void on_spinBoxCrazy_editingFinished();
        void slotMaestroSweep();
        void slotMaestroCrazy();
        void on_comboBoxSmooth_currentIndexChanged(const QString &arg1);
};

/**************************************************************************************************/

#endif // MAINWINDOW_H
