/*

    Pololu Compact Protocol Cmds

 */
#include "mainwindow.h"
#include <QThread>
#include <QTime>

void delayMs( int millisecondsToWait )
{
    QTime dieTime = QTime::currentTime().addMSecs( millisecondsToWait );
    while( QTime::currentTime() < dieTime ) {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
        }
}


/*
    Sends all servos to their home positions

 Go Home
    Compact protocol: 0xA2
    Pololu protocol: 0xAA, device number, 0x22
*/
void MainWindow::MaestroGoHome()
{
    qDebug() << "\n" << Q_FUNC_INFO;

     QByteArray ba;
     ba[0] = static_cast<char>(0xA2);         //Compact Protocol CMD
     if(mSerialPort->isWritable()) {
         baLastCmd = ba;
         mSerialPort->write(baLastCmd.data());
         }
}

/*
 Get Errors
    Compact protocol: 0xA1
    Pololu protocol: 0xAA, device number, 0x21
    Response: error bits 0-7, error bits 8-15

 */
int16_t MainWindow::MaestroGetError()
{
    qDebug() << "\n" << Q_FUNC_INFO;

    QByteArray ba;
    int16_t error=0;

    ba[0] = static_cast<char>(0xA1);         //Compact Protocol CMD
    if(mSerialPort->isWritable()) {
        baLastCmd = ba;
        baRespose.clear();
        mSerialPort->write(baLastCmd.data());
        for (int i=0;i<100;i++){ //max 1000ms
            delayMs(10);
            if (baRespose.size()>0){
                qDebug() <<"Resp:0x" << baRespose.toHex();
                error = baRespose[0]+baRespose[1]*256;
                break;
                }
            }
        }
    return error;
}


/*
   Returns the position of the specified servo in microseconds

        Note that if the microcontroller is unable to read from the servo for any reason than Null or some other obscure value could be returned.

        If the channel is configured as an input, the position represents the voltage measured on the channel.
    The inputs on channels 0–11 are analog: their values range from 0 to 1023, representing voltages from 0 to 5 V.
    The inputs on channels 12–23 are digital: their values are either exactly 0 or exactly 1023.



Get Position
    Compact protocol: 0x90, channel number
    Pololu protocol: 0xAA, device number, 0x10, channel number
    Response: position low 8 bits, position high 8 bits
*/
int16_t MainWindow::MaestroGetPositionUs(int8_t ch)
{
    qDebug() << "\n" << Q_FUNC_INFO;

    QByteArray ba;
    int16_t pos=0, steps=0;

    ba[0] = static_cast<char>(0x90);         //Compact Protocol CMD
    ba[1] = static_cast<char>(ch);           //Channel number
    if(mSerialPort->isWritable()) {
        baLastCmd = ba;
        //mSerialPort->write(baLastCmd.data()); //will stop after 0x00
        long long iWritten = mSerialPort->write(baLastCmd.data(), 2);
        qDebug() << "MaestroGetPosion:" << baLastCmd.toHex() << iWritten;

        for (int i=0;i<100;i++){ //max 1000ms
            delayMs(10);
            if (baRespose.size()>0){
                qDebug() << "Maestro GotPosion Steps:" << baRespose.toHex();
                unsigned char cLSB = static_cast<unsigned char>(baRespose[0]);
                unsigned char cMSB = static_cast<unsigned char>(baRespose[1]);
                //qDebug() << QString::number(iLSB,16) << QString::number(iMSB, 16);
                steps = (cMSB*256)+cLSB;
                qDebug() << "GotPostion steps:" << steps;
                pos = steps/4;    //convert steps back to us
                qDebug() << "GotPosition   us:" << pos;
                break;
                }
            }

        }
    return pos;
}

/*
Set Target
    Compact protocol: 0x84, channel number, target low bits, target high bits
    Pololu protocol: 0xAA, device number, 0x04, channel number, target low bits, target high bits

    unsigned char command[6] = { 0xAA, deviceNumber, 0x84 & 0x7F, channelNumber, target & 0x7F, (target >> 7) & 0x7F };

        The lower 7 bits of the third data byte represent bits 0–6 of the target (the lower 7 bits),
    while the lower 7 bits of the fourth data byte represent bits 7–13 of the target. The target is a non-negative integer.

        If the channel is configured as a SERVO, then the target represents the pulse width to transmit in units of quarter-microseconds.
    A target value of 0 tells the Maestro to stop sending pulses to the servo.
        If the channel is configured as a digital OUTPUT, values less than 6000 tell the Maestro to drive the line low,
    while values of 6000 or greater tell the Maestro to drive the line high.

        For example, if channel 2 is configured as a servo and you want to set its target to 1500 µs
    (1500×4 = 6000 = 010111 01110000 in binary), you could send the following byte sequence:

    in binary: 10000100, 00000010, 01110000, 00101110
    in hex: 0x84, 0x02, 0x70, 0x2E
    in decimal: 132, 2, 112, 46

*/
void MainWindow::MaestroSetTargetUs(int8_t ch, int16_t us)
{
    qDebug() << "\n" << Q_FUNC_INFO;
    if ((us<mChannelMin) || (us>mChannelMax)) return;

    int steps = us*4; //convert us to steps (4steps=1us)
    QByteArray ba;
    ba[0] = static_cast<char>(0x84);         //Compact Protocol CMD
    ba[1] = static_cast<char>(ch);           //Channel number
    ba[2] = static_cast<char>(steps & 0x7F);   //LSB
    ba[3] = static_cast<char>((steps >> 7) & 0x7F);     //MSB
    if(mSerialPort->isWritable()) {
        baLastCmd = ba;
        long long iWritten = mSerialPort->write(baLastCmd.data(), 4);
        qDebug() << "MaestroSetTarget steps:" << steps << baLastCmd.toHex() << iWritten;
        qDebug() << "Position us:" << us;
        }
    delayMs(100); //allow some delay for smoothing delay
}

/*
Set Speed
Compact protocol: 0x87, channel number, speed low bits, speed high bits
Pololu protocol: 0xAA, device number, 0x07, channel number, speed low bits, speed high bits

    This command limits the speed at which a servo channel’s output value changes.
The speed limit is given in units of (0.25 μs)/(10 ms), except in special cases (see Section 4.b).
    For example, the command 0x87, 0x05, 0x0C, 0x01
sets the speed of servo channel 5 to a value of 140, which corresponds to a speed of 3.5 μs/ms.
 */
void MainWindow::MaestroSetSpeed(int8_t ch, int16_t speed)
{
    qDebug() << "\n" << Q_FUNC_INFO;

    QByteArray ba;
    ba[0] = static_cast<char>(0x87);         //Compact Protocol CMD
    ba[1] = static_cast<char>(ch);           //Channel number
    ba[2] = static_cast<char>(speed & 0x7F);   //LSB
    ba[3] = static_cast<char>((speed >> 7) & 0x7F);     //MSB
    if(mSerialPort->isWritable()) {
        baLastCmd = ba;
        //mSerialPort->write(baLastCmd.data()); //will stop after 0x00
        long long iWritten = mSerialPort->write(baLastCmd.data(), 4);
        qDebug() << "MaestroSetSpeed:" << baLastCmd.toHex() << iWritten;
        }
    delayMs(10);
}

/*
Set Acceleration
Compact protocol: 0x89, channel number, acceleration low bits, acceleration high bits
Pololu protocol: 0xAA, device number, 0x09, channel number, acceleration low bits, acceleration high bits

    This command limits the acceleration of a servo channel’s output.
    The acceleration limit is a value from 0 to 255 in units of (0.25 μs)/(10 ms)/(80 ms), except in special cases (see Section 4.b).
    A value of 0 corresponds to no acceleration limit.
 */
void MainWindow::MaestroSetAcc(int8_t ch, int16_t acc)
{
    qDebug() << "\n" << Q_FUNC_INFO;

    QByteArray ba;
    ba[0] = static_cast<char>(0x89);         //Compact Protocol CMD
    ba[1] = static_cast<char>(ch);           //Channel number
    ba[2] = static_cast<char>(acc & 0x7F);   //LSB
    ba[3] = static_cast<char>((acc >> 7) & 0x7F);     //MSB
    if(mSerialPort->isWritable()) {
        baLastCmd = ba;
        //mSerialPort->write(baLastCmd.data()); //will stop after 0x00
        long long iWritten = mSerialPort->write(baLastCmd.data(), 4);
        qDebug() << "MaestroSetAcc:" << baLastCmd.toHex() << iWritten;
        }
    delayMs(10);
}

void MainWindow::on_pushButtonReadError_clicked()
{

    int16_t error = MaestroGetError();
    qDebug() << "error:" << error;

    QString sError = QString::number(0x10000+error,2); //convert to binary with zero leading
    ui->labelError->setText(sError.right(8));
    if (error>0)
        ui->labelError->setStyleSheet("QLabel{color:magenta;}");
    else
        ui->labelError->setStyleSheet("QLabel{color:gray;}");

}

