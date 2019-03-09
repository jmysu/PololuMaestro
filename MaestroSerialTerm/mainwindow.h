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

#include "ui_mainwindow.h"
#include "constants.h"

/**************************************************************************************************/

enum terminal_modes { ASCII, HEX, ASCII_HEX };

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

        static const unsigned short mMaestroChannels  = 12; //Maestro Channels
        static const unsigned short mMaestroSpeedInit = 10; //Maestro Init Speed
        static const unsigned short mMaestroAccInit   = 250;//Maestro Init Accelation
        static const unsigned short mMaestroMinValue  = 0x0F80; //0x0F80
        static const unsigned short mMaestroMaxlValue = 0x1F40; //0x1F40
        //Settings
        void saveSettings();
        void readSettings();
        QString mAppDirPath = QApplication::applicationDirPath();
        bool bSerialOpen = false;

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
        int16_t MaestroGetPosition(char ch);
        void MaestroSetTarget(char ch, int pos);
        void MaestroSetSpeed(char ch, int speed);
        void MaestroSetAcc(char ch, int acc);
        void MaestroTabInit();

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
        void slotTargetSpinBox();
        void slotTargetSlider();
        void slotSpeedSpinBox();
        void slotAccSpinBox();

        void on_pushButtonReadError_clicked();
        void on_pushButtonInit_clicked();
};

/**************************************************************************************************/

#endif // MAINWINDOW_H
