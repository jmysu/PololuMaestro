#include <mainwindow.h>

// Serial Ports check timer initialization and start
void MainWindow::SerialPortsChecks_timer_init(void)
{
    timerSerialPorts = new QTimer(this);
    connect(timerSerialPorts, SIGNAL(timeout()), this, SLOT(SerialPortsCheck()));
    timerSerialPorts->start(500); // Run SerialPortsCheck() each 500ms
}

// Serial Ports check timer fire event handler (this method run each 500ms)
void MainWindow::SerialPortsCheck(void)
{
    //qDebug() << "\n" << Q_FUNC_INFO;

    QStringList slActualPorts;

    // Get all availables Serial Ports and add them to a list
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
        slActualPorts.append(info.portName());

    // Add new detected ports to availables ports list
    for(int i = 0; i < slActualPorts.size(); i++){
        QString port = slActualPorts[i];
        if (!slAvailableSerialPorts.contains(port)){
            slAvailableSerialPorts.append(port);
            ui->comboBox_SerialPort->addItem(port);
            }
        }

    // Remove not detected ports from availables ports list
    QList<int> to_rm_indexes;
    for(int i = 0; i < slAvailableSerialPorts.size(); i++){
        QString port = slAvailableSerialPorts[i];
        if (!slActualPorts.contains(port))
            to_rm_indexes.append(i);
        }
    for(const int &i : to_rm_indexes){
        slAvailableSerialPorts.removeAt(i);
        ui->comboBox_SerialPort->removeItem(i);
        }

    //Check if mSettingsPort available?
    if (ui->comboBox_SerialPort->currentText() != sSettingsPort) {
        int iPortIdx = ui->comboBox_SerialPort->findText(sSettingsPort);
        if (iPortIdx >0) {
            ui->comboBox_SerialPort->setCurrentIndex(iPortIdx);
            qDebug() << "Restored Serial Port Name:" << ui->comboBox_SerialPort->currentText();
            }
        }
    //Update baud from settings
    if (ui->comboBox_bauds->currentText() != sSettingsBaud) {
        int iBaudIdx = ui->comboBox_bauds->findText(sSettingsBaud);
        if( iBaudIdx>=0 ) {
            ui->comboBox_bauds->setCurrentIndex(iBaudIdx);
            qDebug() << "Restored Serial Bauds:" << ui->comboBox_bauds->currentText();
            }
    }

    if ((bSerialOpen) && ui->tabMaestro->isEnabled()){
        //qDebug() << "Check Positions";
        for (int8_t r=0; r<mMaestroChannels; r++){
            if (slSettingsChannelOpt[r] == "Off") continue;
            slotUpdatePosition(r);
            }
        }

}

/* Serial Port Error Signal Handler */
// An error occurred in the port
void MainWindow::SerialPortErrorHandler(void)
{
    if(mSerialPort->error() != QSerialPort::NoError)
    {
        QByteArray qba_error = mSerialPort->errorString().toUtf8();
        debug_print("Error - %s\n", qba_error.data());

        ui->label_status->setStyleSheet("QLabel { color : red; }");
        ui->label_status->setText("Status: " + mSerialPort->errorString());

        // Force close
        CloseSerialPort();
    }
}

// Open Serial port if there is any valid selected port and bauds in comboBoxes
void MainWindow::OpenSerialPort(void)
{
    QString selected_port = ui->comboBox_SerialPort->currentText();
    if((selected_port.isNull()) || (selected_port.isEmpty())){
        debug_print("Port is null or empty");
        return;
        }

    QString selected_bauds = ui->comboBox_bauds->currentText();
    if((selected_bauds.isNull()) || (selected_bauds.isEmpty())){
        debug_print("Bauds is null or empty");
        return;
        }

    QByteArray qba_port = selected_port.toUtf8();
    QByteArray qba_selected_bauds = selected_bauds.toUtf8();
    debug_print("Trying to open port %s at %s bauds.", qba_port.data(), qba_selected_bauds.data());
    mSerialPort->setPortName(selected_port);
    if(mSerialPort->open(QIODevice::ReadWrite)){
        mSerialPort->setDataBits(QSerialPort::Data8);
        mSerialPort->setBaudRate(selected_bauds.toInt());
        mSerialPort->setParity(QSerialPort::NoParity);
        mSerialPort->setStopBits(QSerialPort::OneStop);
        mSerialPort->setFlowControl(QSerialPort::NoFlowControl);

        // Clear possible buffered data
        mSerialPort->clear();

        ui->label_status->setStyleSheet("QLabel { color : blue; }");
        ui->label_status->setText("Status: Connected.");

        // Change enable/disable states of UI elements
        ui->pushButton_close->setEnabled(true);
        ui->pushButton_send->setEnabled(true);
        ui->pushButton_send_Hex->setEnabled(true);
        ui->lineEdit_toSend->setEnabled(true);
        ui->lineEdit_toSend_Hex->setEnabled(true);

        ui->pushButton_open->setEnabled(false);
        ui->comboBox_SerialPort->setEnabled(false);

        if (terminal_mode == ASCII)
            ui->lineEdit_toSend->setFocus();
        else
            ui->lineEdit_toSend_Hex->setFocus();

        debug_print("Port successfully open.");
        this->setWindowTitle(APP_NAME+QString(" V")+APP_VERSION+QString(" ")+selected_port);

        //Display serial info
        QSerialPortInfo info =  QSerialPortInfo(mSerialPort->portName());
        qDebug() << "Name : " << info.portName();
        qDebug() << "Description : " << info.description();
        qDebug() << "Manufacturer: " << info.manufacturer();
        qDebug() << "Serial Number: " << info.serialNumber();
        qDebug() << "System Location: " << info.systemLocation();
        ui->label_serialPort->setText(info.description()+" :#"+info.serialNumber()+" :@"+info.systemLocation());
        ui->tabTerminal->setEnabled(true);
        ui->tabMaestro->setEnabled(true);
        bSerialOpen = true;

        if (ui->tabMaestro->isVisible()){
            ui->comboBoxSmooth->setCurrentIndex(E0_ServoSmooth);
            ui->comboBoxSmooth->currentIndexChanged(ui->comboBoxSmooth->currentText());

            on_pushButtonInit_clicked();
            }
        }
}

// Close serial port if it is open
void MainWindow::CloseSerialPort(void)
{
    // Close the port if it is available
    if((mSerialPort->isWritable()) || (mSerialPort->isReadable())){
        mSerialPort->close();
        debug_print("Port successfully close.");

        ui->label_status->setStyleSheet("QLabel { color : black; }");
        ui->label_status->setText("Status: Disconnected.");
        }

    // Change enable/disable states of UI elements
    ui->pushButton_close->setEnabled(false);
    ui->pushButton_send->setEnabled(false);
    ui->pushButton_send_Hex->setEnabled(false);
    ui->lineEdit_toSend->setEnabled(false);
    ui->lineEdit_toSend_Hex->setEnabled(false);
    ui->pushButton_open->setEnabled(true);
    ui->comboBox_SerialPort->setEnabled(true);
    ui->tabTerminal->setEnabled(false);
    ui->tabMaestro->setEnabled(false);
    this->setWindowTitle("");
    bSerialOpen = false;
}


// Serial send data from lineEdit box
void MainWindow::SerialSendAscii(void)
{
    // Check if to send data box is empty
    QString qstr_to_send = ui->lineEdit_toSend->text();
    if((qstr_to_send.isNull()) || (qstr_to_send.isEmpty())){
        debug_print("Data to send box is null or empty");
        return;
        }

    // Add to send data to history list
    slSendHistory.prepend(qstr_to_send);
    iHistoryIdx = -1;

    // Append selected end character to data to be send
    qstr_to_send = qstr_to_send + EOL_values[ui->comboBox_EOL->currentIndex()];

    // Send the data if the port is available
    if(mSerialPort->isWritable()){
        // Convert QString to QByteArray to send a char* type
        QByteArray qba_to_send = qstr_to_send.toUtf8();
        qDebug() << "Sending Ascii:" << qba_to_send;
        mSerialPort->write(qba_to_send.data());
        }

    ui->lineEdit_toSend->setFocus();
}


/* Serial Sends Hex*/
void MainWindow::SerialSendHex(void)
{
    // Check if to send data box is empty
    QString qstr_to_send = ui->lineEdit_toSend_Hex->text();
    if((qstr_to_send.isNull()) || (qstr_to_send.isEmpty())){
        debug_print("HexData to send box is null or empty");
        return;
        }

    // Add to send data to history list
    slSendHistory.prepend(qstr_to_send);
    iHistoryIdx = -1;

    // Append selected end character to data to be send
    qstr_to_send = qstr_to_send + EOL_values[ui->comboBox_EOL->currentIndex()];

    // Send the data if the port is available
    if(mSerialPort->isWritable()) {
        // Convert HexString to QByteArray to send a char* type
        baLastCmd = QByteArray::fromHex(qstr_to_send.toUtf8());
        long long iWritten = mSerialPort->write(baLastCmd.data(), baLastCmd.size()); //Without size, the NULL will terminate write!!!
        //qDebug() << "Sent Hex:" << baLastCmd.toHex() << "Bytes:" << iWritten;
        QString sHex = baLastCmd.toHex();
        QString sSentHex = QString("Sent Hex:%1 (%2bytes)").arg(sHex).arg(iWritten);
        qDebug() << sSentHex;
        }

    ui->lineEdit_toSend_Hex->setFocus();
}

// Serial received data from port
void MainWindow::SerialReceive(void)
{
    // Send the data if the port is available
    if(mSerialPort->isReadable()){
        baRespose = mSerialPort->readAll();
        qDebug() << "Cmd:"<< baLastCmd << " Response:" << baRespose.toHex();
        PrintReceivedData(ui->textBrowser_serial_Ascii, ui->textBrowser_serial_Hex, terminal_mode);
        }
}
