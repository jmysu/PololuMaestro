#include "mainwindow.h"


/*
MaestroRestoreSettings()


 */
void MainWindow::MaestroRestoreSettings()
{
    qDebug() << "\n" << Q_FUNC_INFO;
    //Restore Channel Options
    for ( unsigned char r=0; r<mMaestroChannels; ++r ){
        QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(r, E0_OutputOpt);
        QWidget *widget = item->widget();
        QComboBox *combobox = dynamic_cast<QComboBox*>(widget);

        int index = combobox->findText(slSettingsChannelOpt[r]);
        if( index>=0 )
            combobox->setCurrentIndex(index);
        qDebug() << r << combobox->currentText();
        }

}
/*
MaestroTabInit()

 */
void MainWindow::MaestroTabInit()
{
    QGridLayout *gridMaestro = ui->gridLayoutMaestro;
    for ( int8_t r=0; r<mMaestroChannels; ++r ){

        QComboBox *comboBox = new QComboBox(this);
        comboBox->setProperty("row",r);
        comboBox->addItems(slOutputOptions);
        gridMaestro->addWidget(comboBox, r, E0_OutputOpt, Qt::AlignLeft);
        connect(comboBox, &QComboBox::currentTextChanged, this, [=](){slotChannelOptChanged();});

        //E0_ChannelNumber Label --------------------------------
        gridMaestro->addWidget( new QLabel(QString("Channel #%1").arg(r), this), r, E1_ChannelNumber, Qt::AlignLeft);
        //E1_Position Label -------------------------------------
        gridMaestro->addWidget( new QLabel("Position", this),   r, E2_Position, Qt::AlignLeft );
        //E2_PositionSpinBox Read Only
        QSpinBox *positionSpinBox = new QSpinBox(this);
        positionSpinBox->setEnabled(false); //Read Only
        gridMaestro->addWidget( positionSpinBox,                r, E3_PositionSpinBox);
        //E3_Target Label ---------------------------------------
        gridMaestro->addWidget( new QLabel("Target", this),     r, E4_Target, Qt::AlignLeft );
        //E4_TargetSpinBox
        QSpinBox *targetSpinBox = new QSpinBox(this);
        targetSpinBox->setProperty("row", r); //add row property
        gridMaestro->addWidget( targetSpinBox,                  r, E5_TargetSpinBox);
        connect(targetSpinBox, &QSpinBox::editingFinished, this, [=](){slotTargetSpinBox();});
        //E5_TargetSlider----------------------------------------
        QSlider *targetSlider = new QSlider(Qt::Horizontal, this);
        targetSlider->setProperty("row", r);  //add row property
        gridMaestro->addWidget( targetSlider,                   r, E6_TargetSlider);
        connect(targetSlider, &QSlider::sliderReleased, this, [=](){slotTargetSlider();});
        //E6_Speed Label ----------------------------------------
        gridMaestro->addWidget( new QLabel("Speed", this),      r, E7_Speed, Qt::AlignLeft );
        //E7_SpeedSpinBox
        QSpinBox *speedSpinBox = new QSpinBox(this);
        speedSpinBox->setProperty("row", r); //add row property
        speedSpinBox->setMinimum(0);
        speedSpinBox->setMaximum(99);
        speedSpinBox->setValue(mMaestroSpeedInit);
        speedSpinBox->setToolTip("0~999\nHit <Enter> to apply!");

        gridMaestro->addWidget( speedSpinBox,                   r, E8_SpeedSpinBox);
        connect(speedSpinBox,  &QSpinBox::editingFinished, this, [=](){slotSpeedSpinBox();});
        //E8_Acc Label ------------------------------------------
        gridMaestro->addWidget( new QLabel("Acceleration",this),r, E9_Acc, Qt::AlignLeft  );
        //E9_AccSpinBox
        QSpinBox *accSpinBox = new QSpinBox(this);
        accSpinBox->setProperty("row", r); //add row property
        accSpinBox->setMinimum(0); //slowest spped on init
        accSpinBox->setMaximum(99);
        accSpinBox->setValue(mMaestroAccInit);
        accSpinBox->setToolTip("0~255\nHit <Enter> to apply!");
        gridMaestro->addWidget( accSpinBox,                     r, E10_AccSpinBox);
        connect(accSpinBox, &QSpinBox::editingFinished, this, [=](){slotAccSpinBox();});

        //Adjust column width
        gridMaestro->setColumnMinimumWidth(E3_PositionSpinBox,  64);
        gridMaestro->setColumnMinimumWidth(E5_TargetSpinBox,    64);
        gridMaestro->setColumnMinimumWidth(E8_SpeedSpinBox,     64);
        gridMaestro->setColumnMinimumWidth(E10_AccSpinBox,      64);
        //Adjust min/max
        //int iMin = mChannelMin+mChannelMinAdj[r];
        //int iMax = mChannelMax+mChannelMaxAdj[r];
        int iMin = MaestroGetChannelMin(r);
        int iMax = MaestroGetChannelMax(r);
        int iMiddle = iMin + (iMax-iMin)/2;
        int iStep= (iMax-iMin)/50;
        int iInterval= (iMax-iMin)/4;
qDebug() << r << iMin << iMiddle << iMax;
        positionSpinBox->setMinimum(iMin);
        targetSpinBox->setMinimum(iMin);
        targetSlider->setMinimum(iMin);

        positionSpinBox->setMaximum(iMax);
        targetSpinBox->setMaximum(iMax);
        targetSlider->setMaximum(iMax);

        positionSpinBox->setSingleStep(iStep);
        targetSpinBox->setSingleStep(iStep);
        targetSlider->setSingleStep(iStep);
        targetSlider->setTickInterval(iInterval);
        targetSlider->setTickPosition(QSlider::TicksAbove);

        targetSlider->setValue(iMiddle);
        targetSpinBox->setValue(iMiddle);
        positionSpinBox->setValue(iMiddle);

        //Disable rows for ChanelOpt=="Off", E2_Posion and E3_PositionSpinBox
        for (int c=1; c<=E10_AccSpinBox; c++) {
            QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(r, c);
            QWidget *widget = item->widget();
            if (slSettingsChannelOpt[r]=="Off" || (c==E2_Position) || (c==E3_PositionSpinBox))
                widget->setEnabled(false);
            }
        }

    //Create timerSweep/timerCrazy
    timerMaestroSweep = new QTimer(this);
    connect(timerMaestroSweep, SIGNAL(timeout()), this, SLOT(slotMaestroSweep()));
    timerMaestroCrazy = new QTimer(this);
    connect(timerMaestroCrazy, SIGNAL(timeout()), this, SLOT(slotMaestroCrazy()));

    //init Servo smooth
    ui->comboBoxSmooth->setCurrentIndex(E0_ServoSmooth); //0:Smooth 1:Fast
    ui->comboBoxSmooth->currentIndexChanged(ui->comboBoxSmooth->currentText());

}

/*
MaestroGetChannelMin(channel)

    get the Min/Max value from channel
    as ther are different min/max for different port setting

    Servo:  1000us/2000us with Adj
    Input:  0/1023
    Output: 0/12000

        If the channel is configured as a digital output, a position value less than 6000 means the Maestro is driving the line low,
    while a position value of 6000 or greater means the Maestro is driving the line high.

        If the channel is configured as an input, the position represents the voltage measured on the channel.
    The inputs on channels 0–11 are analog: their values range from 0 to 1023, representing voltages from 0 to 5 V.
    (The inputs on channels 12–23 are digital: their values are either exactly 0 or exactly 1023.)

 *
 */
int16_t MainWindow::MaestroGetChannelMin(int8_t channel)
{
    int16_t iMinCh=mChannelMin+mChannelMinAdj[channel];
    if (slSettingsChannelOpt[channel]=="Servo")
        iMinCh = mChannelMin+mChannelMinAdj[channel];
    else if (slSettingsChannelOpt[channel]=="Input")
        iMinCh = 0;
    else if (slSettingsChannelOpt[channel]=="Output")
        iMinCh = 0;

    return iMinCh;
}
int16_t MainWindow::MaestroGetChannelMax(int8_t channel)
{
    int16_t iMaxCh=mChannelMax+mChannelMaxAdj[channel];
    if (slSettingsChannelOpt[channel]=="Servo")
        iMaxCh = mChannelMax+mChannelMaxAdj[channel];
    else if (slSettingsChannelOpt[channel]=="Input")
        iMaxCh = 1023;
    else if (slSettingsChannelOpt[channel]=="Output")
        iMaxCh = 12000;

    return iMaxCh;
}
/*
on_pushButtonInit_clicked()


    Maestro GoHome

    Init all TargetPosition, Speed and Acceration

 */
void MainWindow::on_pushButtonInit_clicked()
{
    //MaestroGoHome();  //reset all
    //init Servo smooth
    ui->comboBoxSmooth->setCurrentIndex(E0_ServoSmooth); //0:Smooth 1:Fast
    ui->comboBoxSmooth->currentIndexChanged(ui->comboBoxSmooth->currentText());

    for (int r=0;r<mMaestroChannels;r++) {
        //int iMin = mChannelMin+mChannelMinAdj[r];
        //int iMax = mChannelMax+mChannelMaxAdj[r];
        int iMin = MaestroGetChannelMin(r);
        int iMax = MaestroGetChannelMax(r);
        int iMiddle = iMin + (iMax-iMin)/2;
        //Sync spinbox
        QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(r, E5_TargetSpinBox);
        QWidget *widget = item->widget();
        QSpinBox *spinbox = dynamic_cast<QSpinBox*>(widget);
        spinbox->setValue(iMiddle);
        spinbox->editingFinished();
        }
}

/*
on_comboBoxSmooth_currentIndexChanged(const QString &arg1)

    Maestro servo smooth/fast control mode

 */
void MainWindow::on_comboBoxSmooth_currentIndexChanged(const QString &arg1)
{
    qDebug() << "Maestro Servo Mode:" << arg1;
    if (arg1 == "Smooth") {
        for (int c=0;c<mMaestroChannels;c++) {
            QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(c, E8_SpeedSpinBox);
            QWidget *widget = item->widget();
            QSpinBox *spinbox = dynamic_cast<QSpinBox*>(widget);
            spinbox->setValue(mMaestroSpeedInit);
            spinbox->editingFinished();

            item = ui->gridLayoutMaestro->itemAtPosition(c, E10_AccSpinBox);
            widget = item->widget();
            spinbox = dynamic_cast<QSpinBox*>(widget);
            spinbox->setValue(mMaestroAccInit);
            spinbox->editingFinished();
            }
        }
    else {
        for (int c=0;c<mMaestroChannels;c++) {
            QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(c, E8_SpeedSpinBox);
            QWidget *widget = item->widget();
            QSpinBox *spinbox = dynamic_cast<QSpinBox*>(widget);
            spinbox->setValue(0);
            spinbox->editingFinished();

            item = ui->gridLayoutMaestro->itemAtPosition(c, E10_AccSpinBox);
            widget = item->widget();
            spinbox = dynamic_cast<QSpinBox*>(widget);
            spinbox->setValue(0);
            spinbox->editingFinished();
            }
        }

}


/*
on_spinBoxSweep_editingFinished()

  Sweeping servos timer secs changed
  Stop Crazy timer and rest it's value
  Start Sweep timer
 */
void MainWindow::on_spinBoxSweep_editingFinished()
{
    if (timerMaestroCrazy->isActive()){
        timerMaestroCrazy->stop();
        ui->spinBoxCrazy->setValue(0);
        }

    if (ui->spinBoxSweep->value()==0) {
        timerMaestroSweep->stop();
        qDebug() << "Stopping sweep timer...";
        ui->label_status->setText("Status: servo sweeping stopped!");
        }
    else {
        timerMaestroSweep->start(ui->spinBoxSweep->value()*1000);
        ui->label_status->setText("Status: servo sweeping in action!");
        }
    qDebug() << "timer active:" << timerMaestroSweep->isActive();
}
/*
on_spinBoxCrazy_editingFinished()

  Crazy servos timer secs changed
  Stop Sweeping timer and reset it's value
  Start Crazy timer
 */
void MainWindow::on_spinBoxCrazy_editingFinished()
{
    if (timerMaestroSweep->isActive()) {
        timerMaestroSweep->stop();
        ui->spinBoxSweep->setValue(0);
        }

    if (ui->spinBoxCrazy->value()==0) {
        timerMaestroCrazy->stop();
        qDebug() << "Stopping crazy timer...";
        ui->label_status->setText("Status: servo Crazy-mode stopped!");
        }
    else {
        timerMaestroCrazy->start(ui->spinBoxCrazy->value()*1000);
        ui->label_status->setText("Status: servo Crazy-mode in action!");
        }
    qDebug() << "timer active:" << timerMaestroCrazy->isActive();
}

/*
slotMaestroSweep()

   Servo Sweeping exercise

   Max/Min sweeping exercise servos

 */
void MainWindow::slotMaestroSweep()
{
static bool bSweep=false;

    qDebug() << "\n" << Q_FUNC_INFO;
    qDebug() << GetActualSystemTime();
    bSweep = !bSweep;

    for (int r=0; r<mMaestroChannels; r++) {
        if ( (slSettingsChannelOpt[r]=="Off") || (slSettingsChannelOpt[r]=="Input") )continue; //Skip if Off or Input

        QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(r, E6_TargetSlider);
        QWidget *widget = item->widget();
        QSlider *slider = dynamic_cast<QSlider*>(widget);
        if (bSweep) {
            slider->setValue(mChannelMin+mChannelMinAdj[r]);
            slider->sliderReleased(); //emit signal
            }
        else {
            slider->setValue(mChannelMax+mChannelMaxAdj[r]);
            slider->sliderReleased(); //emit signal
            }
        }
}


/*
slotMaestroCrazy()

   Make servo crazy

   Randomly set active srvo positions

 */
void MainWindow::slotMaestroCrazy()
{
    qDebug() << "\n" << Q_FUNC_INFO;
    qDebug() << GetActualSystemTime();
    //qsrand(0);

    //ui->comboBoxSmooth->setCurrentIndex(E1_ServoFast);
    //ui->comboBoxSmooth->currentIndexChanged(ui->comboBoxSmooth->currentText());

    for (int r=0; r<mMaestroChannels; r++) {
        if ( (slSettingsChannelOpt[r]=="Off") || (slSettingsChannelOpt[r]=="Input") )continue; //Skip if Off or Input

        QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(r, E6_TargetSlider);
        QWidget *widget = item->widget();
        QSlider *slider = dynamic_cast<QSlider*>(widget);

        //Set crazy positions
        //int iMin = mChannelMin+mChannelMinAdj[r];
        //int iMax = mChannelMax+mChannelMaxAdj[r];
        int iMin = MaestroGetChannelMin(r);
        int iMax = MaestroGetChannelMax(r);
        int iRandom = (qrand() % (iMax-iMin)) + iMin;
        qDebug() << "Random:" << iRandom;
        slider->setValue(iRandom);
        slider->sliderReleased(); //emit signal
        }
}

/*
 slotChannelOptChanged()

    Enable/Disable row for ChannelOpt changes

    ***E2,E3_Position always disabled

 */
void MainWindow::slotChannelOptChanged()
{
    qDebug() << "\n" << Q_FUNC_INFO;
    QComboBox *combo = static_cast<QComboBox *>(QObject::sender());
    int8_t row = combo->property("row").toInt();
    //qDebug() << row << combo->currentText();

    //Disable rows for ChanelOpt=="Off"
    for (int c=1; c<=E10_AccSpinBox; c++) {
        QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(row, c);
        QWidget *widget = item->widget();
        if ((combo->currentText()=="Off") || (combo->currentText()=="Input") || (c==E2_Position) || (c==E3_PositionSpinBox) )
            widget->setEnabled(false);
        else
            widget->setEnabled(true);
        //qDebug() << c << widget->isEnabled();
        }

    //Update slSettingsChannelOpt
    slSettingsChannelOpt[row] = combo->currentText();
    //qDebug() << row << slSettingsChannelOpt[row];

    //Update min/max and value at target spinbox
    int iMin = MaestroGetChannelMin(row);
    int iMax = MaestroGetChannelMax(row);
    int iMiddle = iMin + (iMax-iMin)/2;
    QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(row, E5_TargetSpinBox);
    QWidget *widget = item->widget();
    QSpinBox *spinbox = dynamic_cast<QSpinBox*>(widget);
    spinbox->setRange(iMin, iMax);
    if (slSettingsChannelOpt[row]=="Servo")
        spinbox->setValue(iMiddle);
    else
        spinbox->setValue(iMin);
    //Update slider
    item = ui->gridLayoutMaestro->itemAtPosition(row, E6_TargetSlider);
    widget = item->widget();
    QSlider *slider = dynamic_cast<QSlider*>(widget);
    slider->setRange(iMin, iMax);
    if (slSettingsChannelOpt[row]=="Servo")
        slider->setValue(iMiddle);
    else
        slider->setValue(iMin);
    qDebug() << "Chanel new range:" << iMin << iMax;
}

/*
slotUpdatePosition()

    Get Postion and update GUI

 */
void MainWindow::slotUpdatePosition(int8_t ch)
{
    int16_t pos = MaestroGetPositionUs(ch);  //Got postion in steps, need to convert back to us (4steps=1us)
    if (slSettingsChannelOpt[ch]=="Input")
        pos = pos*4; //convert to steps when channel Option is Input

    QLayoutItem* itemPos = ui->gridLayoutMaestro->itemAtPosition(ch, E3_PositionSpinBox);
    QWidget *widgetPos = itemPos->widget();
    QSpinBox *spinboxPos = dynamic_cast<QSpinBox*>(widgetPos);
    spinboxPos->setValue(pos);
}


/*
slotTargetSpinBox()

    For example, if channel 2 is configured as a servo and you want to set its target to 1500 µs
(1500×4 = 6000 = 010111 01110000 in binary), you could send the following byte sequence:
                 LSB>>1  MSB
*/
void MainWindow::slotTargetSpinBox()
{
    if (!bSerialOpen) return;

    QSpinBox *spinbox = static_cast<QSpinBox *>(QObject::sender());
    int row = spinbox->property("row").toInt();
    //qDebug() <<  row <<   spinbox->value();
    char channel = static_cast<char>(row);
    if (slSettingsChannelOpt[channel] == "Off") return; //Off channel, do nothing

    MaestroSetTargetUs(channel, spinbox->value());
    //for debugging bytes sending to Maestro
    unsigned char cLSB = (spinbox->value() & 0x7F00) >> 8;
    unsigned char cMSB = (spinbox->value() & 0x7F);
    qDebug() << "Target:" << spinbox->value() << QString::number(spinbox->value(), 16);
    qDebug() << "LSB(7bit)" << QString::number(cLSB,16) << QString::number(cLSB,2);
    qDebug() << "MSB(7bit)" << QString::number(cMSB,16) << QString::number(cMSB,2);

    int iUs = spinbox->value();
    spinbox->setToolTip(QString::number(iUs)+"us"+"\nHit <Enter> to apply!");
    //Sync slider
    QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(row, E6_TargetSlider);
    QWidget *widget = item->widget();
    QSlider *slider = dynamic_cast<QSlider*>(widget);
    slider->setValue(spinbox->value());
    slider->setToolTip(QString::number(iUs)+"us");
    //Sync position
    slotUpdatePosition(channel);
}

/*
slotTargetSlider()

    Update target position slider and spinbox

 */
void MainWindow::slotTargetSlider()
{
    if (!bSerialOpen) return;

    QSlider *slider = static_cast<QSlider *>(QObject::sender());
    int row = slider->property("row").toInt();
    //qDebug() <<  row <<   slider->value();
    if (slSettingsChannelOpt[row] == "Off") return; //Off channel, do nothing

    //Sync spinbox
    QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(row, E5_TargetSpinBox);
    QWidget *widget = item->widget();
    QSpinBox *spinbox = dynamic_cast<QSpinBox*>(widget);
    spinbox->setValue(slider->value());
    spinbox->editingFinished(); //emit signal
}

/*
slotSpeedSpinBox()

    Update speed spinbox

 */
void MainWindow::slotSpeedSpinBox()
{
    if (!bSerialOpen) return;

    QSpinBox *spinbox = static_cast<QSpinBox *>(QObject::sender());
    int row = spinbox->property("row").toInt();
    //qDebug() <<  row <<   spinbox->value();
    char channel = static_cast<char>(row);
    if (slSettingsChannelOpt[channel] == "Off") return; //Off channel, do nothing

    MaestroSetSpeed(channel, spinbox->value());
}

/*
slotAccSpinBox()

    Update Acceleration spinbox

 */
void MainWindow::slotAccSpinBox()
{
    if (!bSerialOpen) return;

    QSpinBox *spinbox = static_cast<QSpinBox *>(QObject::sender());
    int row = spinbox->property("row").toInt();
    //qDebug() <<  row <<   spinbox->value();
    char channel = static_cast<char>(row);
    if (slSettingsChannelOpt[channel] == "Off") return; //Off channel, do nothing

    MaestroSetAcc(channel, spinbox->value());
}




