#include "mainwindow.h"
//#include "MaestroChannel.h"
/**************************************************************************************************/

/* Main Window Constructor & Destructor */

// Main Window Constructor
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    debug_print("\nApp start.\n");
    debug_print("Setup process start.");
    debug_print("Initializing App Window...");
    ui->setupUi(this);
    ui->lineEdit_toSend->installEventFilter(this);
    ui->lineEdit_toSend_Hex->installEventFilter(this);

    debug_print("Setting initial UI elements values...");
    // ComboBox_EOL initialization
    ui->comboBox_EOL->addItems(comboBox_EOL_values);
    ui->comboBox_EOL->setCurrentIndex(eol_default_value);
    // ComboBox_bauds initialization
    ui->comboBox_bauds->addItems(comboBox_bauds_values);
    ui->comboBox_bauds->setCurrentIndex(bauds_default_value);
    ui->comboBox_bauds->setValidator(new QIntValidator(0, 99999999, this));

    terminal_mode = ASCII;
    /////////// terminal_mode & baud rate restoring
    debug_print("Restoring Settings");
    readSettings();

    // TextBrowsers initialization
    QFont font0("Courier");
    font0.setKerning(false);
    font0.setPointSize(10);
    QFont font1("Courier");
    font1.setKerning(false);
    font1.setPointSize(10);

    ui->textBrowser_serial_Ascii->setFont(font0);
    ui->textBrowser_serial_Hex->setFont(font1);
    ui->textBrowser_serial_Ascii->clear();
    ui->textBrowser_serial_Hex->clear();
    ui->textBrowser_serial_Hex->hide();

    debug_print("Connecting events signals...");

    // UI elements user interactions signals event setup
    connect(ui->pushButton_open,    SIGNAL(released()), this, SLOT(ButtonOpenPressed()));
    connect(ui->pushButton_close,   SIGNAL(released()), this, SLOT(ButtonClosePressed()));
    connect(ui->pushButton_clear,   SIGNAL(released()), this, SLOT(ButtonClearPressed()));
    connect(ui->pushButton_send,    SIGNAL(released()), this, SLOT(ButtonSendAsciiPressed()));
    connect(ui->pushButton_send_Hex,SIGNAL(released()), this, SLOT(ButtonSendHexPressed()));
    connect(ui->lineEdit_toSend,    SIGNAL(returnPressed()), this, SLOT(ButtonSendAsciiPressed()));
    connect(ui->lineEdit_toSend_Hex,SIGNAL(returnPressed()), this, SLOT(ButtonSendHexPressed()));
    connect(ui->comboBox_bauds,     SIGNAL(currentIndexChanged(const QString &)), this, SLOT(CBoxBaudsChanged()));
    connect(ui->comboBox_EOL,       SIGNAL(currentIndexChanged(const QString &)), this, SLOT(CBoxEOLChanged()));
    connect(ui->actionExit,         SIGNAL(triggered()), this, SLOT(MenuBarExitClick()));
    connect(ui->actionASCII_Terminal, SIGNAL(triggered()), this, SLOT(MenuBarTermAsciiClick()));
    connect(ui->actionHEX_Terminal, SIGNAL(triggered()), this, SLOT(MenuBarTermHexClick()));
    connect(ui->actionBoth_ASCII_HEX_Terminals, SIGNAL(triggered()), this, SLOT(MenuBarTermBothAsciiHexClick()));
    connect(ui->actionEnable_Disable_Timestamp, SIGNAL(triggered()), this, SLOT(MenuBarTimestampClick()));
    connect(ui->action_Timestamp_with_ms, SIGNAL(triggered()), this, SLOT(MenuBarTimestampMsClick()));
    connect(ui->actionAbout,        SIGNAL(triggered()), this, SLOT(MenuBarAboutClick()));

    // Connect TextBrowsers Scrolls (scroll one of them move the other)
    static int current_slider_pos = 0;
    QScrollBar* scroll_ascii = ui->textBrowser_serial_Ascii->verticalScrollBar();
    QScrollBar* scroll_hex = ui->textBrowser_serial_Hex->verticalScrollBar();
    connect(scroll_ascii, &QAbstractSlider::valueChanged, [=](int scroll_ascii_slider_pos) {
                if(scroll_ascii_slider_pos != current_slider_pos){
                    scroll_hex->setValue(scroll_ascii_slider_pos);
                    current_slider_pos = scroll_ascii_slider_pos;
                    }
                });
    connect(scroll_hex, &QAbstractSlider::valueChanged, [=](int scroll_hex_slider_pos){
                if(scroll_hex_slider_pos != current_slider_pos){
                    scroll_ascii->setValue(scroll_hex_slider_pos);
                    current_slider_pos = scroll_hex_slider_pos;
                    }
                });

    // Set initial configs to default values
    bTimeStampOn = false;
    bTimeStampMS = false;
    terminal_mode = ASCII_HEX;
    MenuBarTermBothAsciiHexClick();     //Apply menu
    ui->lineEdit_toSend_Hex->setFocus();

    // Instantiate SerialPort object and connect received data and error signal to event handlers
    mSerialPort = new QSerialPort;
    connect(mSerialPort, SIGNAL(readyRead()), this, SLOT(SerialReceive()));
    connect(mSerialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::errorOccurred), this, [=](){SerialPortErrorHandler();});

    // Setup and start timer for Serial Ports checks
    debug_print("Initializing Serial Ports check timer...");
    SerialPortsChecks_timer_init();

    // Add an event filter to catch keyboard signals with an eventFilter() handler
    //installEventFilter(this);

    // Inicial send history index to -1
    iHistoryIdx = -1;
    debug_print("Setup process end.\n");

    //Add Maestro Channels UI
    MaestroTabInit();
    ui->tabTerminal->setEnabled(false);
    ui->tabMaestro->setEnabled(false);
}

// Main Window Destructor
MainWindow::~MainWindow()
{
    // If the timer exists and is active, stop and release it
    if(timerSerialPorts && timerSerialPorts->isActive()){
        timerSerialPorts->stop();

        delete timerSerialPorts;
        timerSerialPorts = nullptr;
        }

    // If the SerialPort Object is not null, close and release it
    if(mSerialPort){
        if((mSerialPort->isWritable()) || (mSerialPort->isReadable())){
            mSerialPort->close();
            debug_print("Port successfully close.");
            }

        delete mSerialPort;
        mSerialPort = nullptr;
        }
    //save Settings
    saveSettings();

    // Release UI
    delete ui;

    debug_print("\nApp end.");
}

/**************************************************************************************************/


/**************************************************************************************************/

/* Serial Port Open & Close */

// Button Open pressed event handler
void MainWindow::ButtonOpenPressed(void)
{
    debug_print("Open Button pressed.");
    OpenSerialPort();
}

// Button Close pressed event handler
void MainWindow::ButtonClosePressed(void)
{
    debug_print("Close Button pressed.");
    CloseSerialPort();
}


/**************************************************************************************************/

/* Bauds and Send EOL bytes change */

// ComboBox Bauds change event handler
void MainWindow::CBoxBaudsChanged(void)
{
    debug_print("Bauds changed.");

    // Close the port if it is already open
    if((mSerialPort->isWritable()) || (mSerialPort->isReadable())){
        debug_print("Reconnecting to port using new baudrate.");
        CloseSerialPort();
        OpenSerialPort();
    }
}

// ComboBox EOL change event handler
void MainWindow::CBoxEOLChanged(void)
{
    debug_print("Send EOL changed.");

    if (terminal_mode == ASCII)
        ui->lineEdit_toSend->setFocus();
    else
        ui->lineEdit_toSend_Hex->setFocus();
}

/**************************************************************************************************/

/* Serial Receive */

// Button clear event handler
void MainWindow::ButtonClearPressed(void)
{
    debug_print("Clear Button pressed.");
    ui->textBrowser_serial_Ascii->clear();
    ui->textBrowser_serial_Hex->clear();

    if (terminal_mode == ASCII)
        ui->lineEdit_toSend->setFocus();
    else
        ui->lineEdit_toSend_Hex->setFocus();

}


// Print the received serial data
void MainWindow::PrintReceivedData(QTextBrowser* textBrowser0, QTextBrowser *textBrowser1, terminal_modes mode)
{
    static uint32_t hex_line_num_chars = 0;

    // First thing to do is get actual time
    QByteArray qba_time;
    if(bTimeStampOn)
        qba_time = QString("[" + GetActualSystemTime() + "] ").toUtf8();

    // EOL in QByteArray
    QByteArray qba_eol = QString("\n").toUtf8();

    // If terminal mode is ASCII or HEX (Single TextBrowser)
    if((mode == ASCII) || (mode == HEX)){
        // Get original cursor and scroll position
        QTextCursor original_cursor_pos = textBrowser0->textCursor();
        int original_scroll_pos = textBrowser0->verticalScrollBar()->value();

        // Set textbox cursor to bottom
        QTextCursor new_cursor = original_cursor_pos;
        new_cursor.movePosition(QTextCursor::End);
        textBrowser0->setTextCursor(new_cursor);

        // Get the received data and split it by lines
        //QByteArray serial_data = mSerialPort->readAll();
        QByteArray serial_data = baRespose;

        QList<QByteArray> lines = serial_data.split('\n');
        //debug_print("Received data: %s", serial_data.toHex().data());

        // Get data last character
        char data_last_char = serial_data[serial_data.size()-1];
        static bool last_line_was_eol = false;

        if(mode == ASCII){
            // For each line of received data
            int num_lines = lines.length();
            for(int i = 0; i < num_lines; i++){
                // Get the ASCII data line
                QByteArray to_print_ascii = lines[i];
                to_print_ascii.replace('\0', '.'); //replace 0x00 with '.'

                // Remove carriage return characters
                QString qstr_to_print_ascii(to_print_ascii);
                qstr_to_print_ascii = qstr_to_print_ascii.remove(QChar('\r'));

                // Add time to data if it is not the first line
                if(i != 0)
                    qstr_to_print_ascii = qstr_to_print_ascii.prepend(qba_time);
                else{
                    // Add time to data if the last written line has an end of line
                    if(last_line_was_eol)
                        qstr_to_print_ascii = qstr_to_print_ascii.prepend(qba_time);
                    else
                        last_line_was_eol = false;
                    }

                // Recover lost EOL due to split if it is not the last line
                bool ignore_last_split_line = false;
                if(i < num_lines-1)
                    qstr_to_print_ascii = qstr_to_print_ascii.append(qba_eol);
                else{
                    // Data last character is an EOL
                    if(data_last_char == '\n'){
                        ignore_last_split_line = true;
                        last_line_was_eol = true;
                        }
                    else
                        last_line_was_eol = false;
                    }

                // Ignore print this line
                if(!ignore_last_split_line){
                    // Convert all non-basic printable ASCII characters to ? symbol (ASCII 63)
                    ushort ascii_num;
                    for(int i = 0; i < qstr_to_print_ascii.length(); i++){
                        ascii_num = qstr_to_print_ascii[i].unicode();
                        if((ascii_num < 32) || (ascii_num > 126)){
                            // If actual char is not end of line ('\n')
                            if(ascii_num != 10)
                                qstr_to_print_ascii[i] = 63; //?
                            }
                        }

                    // Write data line to textbox
                    textBrowser0->insertPlainText(qstr_to_print_ascii);

                    // If Autoscroll is checked, scroll to bottom
                    QScrollBar *vertical_bar = textBrowser0->verticalScrollBar();
                    if(ui->checkBox_autoScroll->isChecked())
                        vertical_bar->setValue(vertical_bar->maximum());
                    else{
                        // Return to previous cursor and scroll position
                        textBrowser0->setTextCursor(original_cursor_pos);
                        vertical_bar->setValue(original_scroll_pos);
                        }
                    }
                }
        }
        else{
            // For each line of received data
            int num_lines = lines.length();
            for(int i = 0; i < num_lines; i++){
                // Get the HEX data line and format it to string
                QByteArray to_print_hex = lines[i].toHex();
                to_print_hex = to_print_hex.toUpper();

                // If in this line we have yet 2 bytes printed, add a space before new data
                if(hex_line_num_chars % 2)
                    to_print_hex.prepend(' ');

                if(to_print_hex.length() > 2){
                    // Add spaces between pair of bytes of hex data (convert "A2345F" to "A2 34 5F")
                    for(int ii = 2; ii < to_print_hex.length()-1; ii = ii + 3)
                        to_print_hex.insert(ii, " ");
                   to_print_hex.append(' ');
                    }
                else if (to_print_hex.length() == 2){
                    // If there is just one byte and is an EOL, add a space
                    if(lines[i][0] != '\n')
                        to_print_hex.append(' ');
                    }

                // Add time to data if it is not the first line
                if(i != 0)
                    to_print_hex = to_print_hex.prepend(qba_time);
                else{
                    // Add time to data if the last written line has an end of line
                    if(last_line_was_eol)
                        to_print_hex = to_print_hex.prepend(qba_time);
                    else
                        last_line_was_eol = false;
                    }

                // Recover lost EOL due to split if it is not the last line
                bool ignore_last_split_line = false;
                if(i < num_lines-1){
                    to_print_hex = to_print_hex.append("0A" + qba_eol);
                    hex_line_num_chars = 0;
                    }
                else{
                    // Data last character is an EOL
                    if(data_last_char == '\n'){
                        ignore_last_split_line = true;
                        last_line_was_eol = true;
                        }
                    else
                        last_line_was_eol = false;
                    }

                // Ignore print this line
                if(!ignore_last_split_line){
                    // Write data line to textbox
                    textBrowser0->insertPlainText(to_print_hex);

                    // If Autoscroll is checked, scroll to bottom
                    QScrollBar *vertical_bar = textBrowser0->verticalScrollBar();
                    if(ui->checkBox_autoScroll->isChecked())
                        vertical_bar->setValue(vertical_bar->maximum());
                    else{
                        // Return to previous cursor and scroll position
                        textBrowser0->setTextCursor(original_cursor_pos);
                        vertical_bar->setValue(original_scroll_pos);
                        }
                    }
                }
            }
    }
    else if (mode == ASCII_HEX){
        // Get original cursor and scroll position
        QTextCursor original_cursor_pos_ascii = textBrowser0->textCursor();
        QTextCursor original_cursor_pos_hex = textBrowser1->textCursor();
        int original_scroll_pos_ascii = textBrowser0->verticalScrollBar()->value();
        int original_scroll_pos_hex = textBrowser1->verticalScrollBar()->value();

        // Set textbox cursor to bottom
        QTextCursor new_cursor_ascii = original_cursor_pos_ascii;
        QTextCursor new_cursor_hex = original_cursor_pos_hex;
        new_cursor_ascii.movePosition(QTextCursor::End);
        new_cursor_hex.movePosition(QTextCursor::End);
        textBrowser0->setTextCursor(new_cursor_ascii);
        textBrowser1->setTextCursor(new_cursor_hex);

        // Get the received data and split it by lines
        //QByteArray serial_data = mSerialPort->readAll();
        QByteArray serial_data = baRespose;

        QList<QByteArray> lines = serial_data.split('\n');

        // Get data last character
        char data_last_char = serial_data[serial_data.size()-1];
        static bool last_line_was_eol = true;

        // For each line of received data
        int num_lines = lines.length();
        for(int i = 0; i < num_lines; i++){
            // Get the ASCII data line
            QByteArray to_print_ascii = lines[i];
            to_print_ascii.replace('\0', '.'); //replace 0x00 with '.'

            // Remove carriage return characters
            QString qstr_to_print_ascii(to_print_ascii);
            qstr_to_print_ascii = qstr_to_print_ascii.remove(QChar('\r'));

            // Get the HEX data line and format it to string
            QByteArray to_print_hex = lines[i].toHex();
            to_print_hex = to_print_hex.toUpper();
            if(to_print_hex.length() > 2){
                // Add spaces between pair of bytes of hex data (convert "A2345F" to "A2 34 5F")
                for(int ii = 2; ii < to_print_hex.length()-1; ii = ii + 3)
                    to_print_hex.insert(ii, " ");
                to_print_hex.append(' ');
                }
             else if (to_print_hex.length() == 2) {
                 // If there is just one byte and is an EOL, add a space
                 if(lines[i][0] != '\n')
                     to_print_hex.append(' ');
                }

            // Add time to data if it is not the first line
            if(i != 0) {
                qstr_to_print_ascii = qstr_to_print_ascii.prepend(qba_time);
                to_print_hex = to_print_hex.prepend(qba_time);
                }
            else{
                // Add time to data if the last written line has an end of line
                if(last_line_was_eol){
                    qstr_to_print_ascii = qstr_to_print_ascii.prepend(qba_time);
                    to_print_hex = to_print_hex.prepend(qba_time);
                    }
                else
                    last_line_was_eol = false;
                }

            bool ignore_last_split_line = false;
            // Recover lost EOL due to split if it is not the last line
            if(i < num_lines-1){
                qstr_to_print_ascii = qstr_to_print_ascii.append(qba_eol);
                to_print_hex = to_print_hex.append("0A" + qba_eol);
                }
            else{
                // Data last character is an EOL
                if(data_last_char == '\n'){
                    ignore_last_split_line = true;
                    last_line_was_eol = true;
                    }
                else
                    last_line_was_eol = false;
                }

            // Ignore print this line
            if(!ignore_last_split_line){
                // Convert all non-basic printable ASCII characters to ? symbol (ASCII 63)
                ushort ascii_num;
                for(int i = 0; i < qstr_to_print_ascii.length(); i++){
                    ascii_num = qstr_to_print_ascii[i].unicode();
                    if((ascii_num < 32) || (ascii_num > 126)){
                        // If actual char is not end of line ('\n')
                        if(ascii_num != 10)
                            qstr_to_print_ascii[i] = 63; //?
                        }
                    }

                // Write data line to ASCII and HEX textboxes
                textBrowser0->insertPlainText(qstr_to_print_ascii);
                textBrowser1->insertPlainText(to_print_hex);

                // If Autoscroll is checked, scroll to bottom
                QScrollBar *vertical_bar_ascii = textBrowser0->verticalScrollBar();
                QScrollBar *vertical_bar_hex = textBrowser1->verticalScrollBar();
                if(ui->checkBox_autoScroll->isChecked()){
                    vertical_bar_ascii->setValue(vertical_bar_ascii->maximum());
                    vertical_bar_hex->setValue(vertical_bar_hex->maximum());
                    }
                else{
                    // Return to previous cursor and scroll position
                    textBrowser0->setTextCursor(original_cursor_pos_ascii);
                    textBrowser1->setTextCursor(original_cursor_pos_hex);
                    vertical_bar_ascii->setValue(original_scroll_pos_ascii);
                    vertical_bar_hex->setValue(original_scroll_pos_hex);
                    }
                }
            }
        }
}

// Get an string of actual system time
QString MainWindow::GetActualSystemTime(void)
{
    QString time;

    // Get local date
    QDateTime date_local(QDateTime::currentDateTime());

    // This is how to get UTC instead local
    /*QDateTime date_UTC(QDateTime::currentDateTime());
    date_UTC.setTimeSpec(Qt::UTC);*/

    time = date_local.time().toString();

    if(bTimeStampMS){
        int i_ms = date_local.time().msec();
        QString str_ms;

        if(i_ms < 10)
            str_ms = "00" + QString::number(i_ms);
        else if (i_ms < 100)
            str_ms = "0" + QString::number(i_ms);
        else
            str_ms = QString::number(i_ms);

        time = time + ":" + str_ms;
        }

    return time;
}

/**************************************************************************************************/

// Button Send pressed event handler
void MainWindow::ButtonSendAsciiPressed(void)
{
    debug_print("Send Button pressed.");
    SerialSendAscii();
    ui->lineEdit_toSend->clear();
    ui->lineEdit_toSend->setFocus();
}
void MainWindow::ButtonSendHexPressed(void)
{
    debug_print("SendHex Button pressed.");
    SerialSendHex();
    ui->lineEdit_toSend_Hex->clear();
    ui->lineEdit_toSend_Hex->setFocus();
}


/**************************************************************************************************/

/* History Change Of To Send Data */

// Keyboard keys pressed event handler
bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    // If Event is a keyboard key press
    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        // If Key pressed is Up arrow, the focus is set to send box and history is not empty
        if(keyEvent->key() == Qt::Key_Up){

            if(ui->lineEdit_toSend->hasFocus()){
                if(!slSendHistory.isEmpty() && (iHistoryIdx >= -1)){
                    if(iHistoryIdx < slSendHistory.size() - 1){
                        iHistoryIdx = iHistoryIdx + 1;
                        ui->lineEdit_toSend->setText(slSendHistory[iHistoryIdx]);
                        }
                    }
                }
            if(ui->lineEdit_toSend_Hex->hasFocus()){
                if(!slSendHistory.isEmpty() && (iHistoryIdx >= -1)){
                    if(iHistoryIdx < slSendHistory.size() - 1){
                        iHistoryIdx = iHistoryIdx + 1;
                        ui->lineEdit_toSend_Hex->setText(slSendHistory[iHistoryIdx]);
                        }
                    }
                }

        }

        // If Key pressed is Down arrow, the focus is set to send box and history is not empty
        if(keyEvent->key() == Qt::Key_Down){

            if(ui->lineEdit_toSend->hasFocus()){
                if(!slSendHistory.isEmpty()){
                    if(iHistoryIdx > -1)
                        iHistoryIdx = iHistoryIdx - 1;
                    if(iHistoryIdx > -1)
                        ui->lineEdit_toSend->setText(slSendHistory[iHistoryIdx]);
                    else
                        ui->lineEdit_toSend->clear();
                    }
                }
            if(ui->lineEdit_toSend_Hex->hasFocus()){
                if(!slSendHistory.isEmpty()){
                    if(iHistoryIdx > -1)
                        iHistoryIdx = iHistoryIdx - 1;
                    if(iHistoryIdx > -1)
                        ui->lineEdit_toSend_Hex->setText(slSendHistory[iHistoryIdx]);
                    else
                        ui->lineEdit_toSend_Hex->clear();
                    }
                }
            }
    }

    return QObject::eventFilter(target, event);
}

/**************************************************************************************************/


/**************************************************************************************************/

/* Menu Bar Functions */

void MainWindow::MenuBarExitClick(void)
{
    QApplication::quit();
}

void MainWindow::MenuBarTermAsciiClick(void)
{
    terminal_mode = ASCII;
    ui->actionASCII_Terminal->setEnabled(false);

    ui->actionHEX_Terminal->setEnabled(true);
    ui->actionBoth_ASCII_HEX_Terminals->setEnabled(true);
    ui->actionHEX_Terminal->setChecked(false);
    ui->actionBoth_ASCII_HEX_Terminals->setChecked(false);

    // Hide the second textBrowser
    ui->textBrowser_serial_Hex->hide();
}

void MainWindow::MenuBarTermHexClick(void)
{
    terminal_mode = HEX;
    ui->actionHEX_Terminal->setEnabled(false);

    ui->actionASCII_Terminal->setEnabled(true);
    ui->actionBoth_ASCII_HEX_Terminals->setEnabled(true);
    ui->actionASCII_Terminal->setChecked(false);
    ui->actionBoth_ASCII_HEX_Terminals->setChecked(false);

    // Hide the second textBrowser
    ui->textBrowser_serial_Hex->hide();
}

void MainWindow::MenuBarTermBothAsciiHexClick(void)
{
    terminal_mode = ASCII_HEX;
    ui->actionBoth_ASCII_HEX_Terminals->setEnabled(false);

    ui->actionASCII_Terminal->setEnabled(true);
    ui->actionHEX_Terminal->setEnabled(true);
    ui->actionASCII_Terminal->setChecked(false);
    ui->actionHEX_Terminal->setChecked(false);

    // Show the second textBrowser and set cursor and scroll position same as first textBrowser
    QString eols;
    size_t cursor_pos = static_cast<size_t>(ui->textBrowser_serial_Ascii->document()->lineCount()-1);
    for(size_t i = 0; i < cursor_pos; i++)
        eols.append("\n");
    ui->textBrowser_serial_Hex->setPlainText(eols);
    QTextCursor cursor = ui->textBrowser_serial_Hex->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textBrowser_serial_Hex->setTextCursor(cursor);
    int scroll_pos = ui->textBrowser_serial_Ascii->verticalScrollBar()->value();
    QScrollBar *vertical_bar = ui->textBrowser_serial_Hex->verticalScrollBar();
    if(ui->checkBox_autoScroll->isChecked())
        vertical_bar->setValue(vertical_bar->maximum());
    else
        vertical_bar->setValue(scroll_pos);
    ui->textBrowser_serial_Hex->show();
}

void MainWindow::MenuBarTimestampClick(void)
{
    bTimeStampOn = !bTimeStampOn;

    if(bTimeStampOn)
        ui->action_Timestamp_with_ms->setEnabled(true);
    else{
        ui->action_Timestamp_with_ms->setChecked(false);
        ui->action_Timestamp_with_ms->setEnabled(false);
        bTimeStampMS = false;
        }
}

void MainWindow::MenuBarTimestampMsClick(void)
{
    bTimeStampMS = !bTimeStampMS;
}

void MainWindow::MenuBarAboutClick(void)
{
    // Create a Dialog who is child of mainwindow
    QDialog *dialog = new QDialog(this, Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    dialog->setWindowTitle("About Maestro Serial Term");
    dialog->setMinimumSize(QSize(480, 280));
    dialog->setMaximumSize(QSize(480, 280));

    QString strAboutOS =  "<br/><br/><br/> <br>OS: "+QSysInfo::prettyProductName();

    // Add Dialog about text
    QLabel* label = new QLabel(dialog);
    label->setTextFormat(Qt::RichText);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setOpenExternalLinks(true);
    label->setText(ABOUT_TEXT+strAboutOS);
    QHBoxLayout* layout = new QHBoxLayout(dialog);
    layout->setContentsMargins(30,20,30,30);
    layout->addWidget(label);
    dialog->setLayout(layout);

    // Execute the Dialog (show() instead dont block parent user interactions)
    dialog->exec();
}
