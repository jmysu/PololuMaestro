#ifndef CONSTANTS_H
#define CONSTANTS_H

/**************************************************************************************************/

#include <QStringList>

/**************************************************************************************************/

/* Debug Enable/Disable and Macro */

#define DEBUG true

#define debug_print(...) do { if(DEBUG) qDebug(__VA_ARGS__); } while (0)

/**************************************************************************************************/

QStringList static comboBox_EOL_values =
{
    "No Line Ending",
    "New Line",
    "Carriage Return",
    "Both NL & CR"
};
QStringList static EOL_values =
{
    "",
    "\n",
    "\r",
    "\r\n"
};
const int eol_default_value = 1; // 1 = New Line

QStringList static comboBox_bauds_values =
{
    "300",
    "1200",
    "2400",
    "4800",
    "9600",
    "19200",
    "38400",
    "57600",
    "74880",
    "115200",
    "230400",
    "250000",
    "500000",
    "1000000",
    "2000000"
};
const int bauds_default_value = 9; // 9 = 115200

const QString ABOUT_TEXT =
    "Maestro Serial Term is based on Qt5 QSerialPort, works for macOS, Linux and Windows.<br/>  " \
    "Currently supports Maestro Compact Protocol.<br/><br/><br/>" \
    "QSerial Terminal Reference: <a href=\"https://github.com/J-Rios/SUSTerm\"> Github repo.</a>" \
    "<br/>" \
    "C++ Masestro Reference: <a href=\"https://github.com/jbitoniau/RapaPololuMaestro\"> Github repo. </a>" \
    "<br/>" \
    "Jimmy's Repository: <a href=\"https://github.com/jmysu/PololuMaestro\"> my Github repo.</a>" \
    ;

/**************************************************************************************************/
enum terminal_modes { ASCII, HEX, ASCII_HEX };

enum Maestro {
    E0_OutputOpt,
    E1_ChannelNumber,
    E2_Position,
    E3_PositionSpinBox,
    E4_Target,
    E5_TargetSpinBox,
    E6_TargetSlider,
    E7_Speed,
    E8_SpeedSpinBox,
    E9_Acc,
    E10_AccSpinBox
};

enum MaestroServoSmooth {
    E0_ServoSmooth,
    E1_ServoFast
};

#endif // CONSTANTS_H
