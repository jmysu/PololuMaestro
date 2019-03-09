#include "mainwindow.h"
//#include "MaestroChannel.h"

void MainWindow::MaestroTabInit()
{
    QGridLayout *gridMaestro = ui->gridLayoutMaestro;
    for ( unsigned char r=0; r<mMaestroChannels; ++r ){
        //E0_ChannelNumber Label --------------------------------
        gridMaestro->addWidget( new QLabel(QString("Channel #%1").arg(r), this), r, E0_ChannelNumber, Qt::AlignLeft);
        //E1_Position Label -------------------------------------
        gridMaestro->addWidget( new QLabel("Position", this),   r, E1_Position, Qt::AlignLeft );
        //E2_PositionSpinBox Read Only
        QSpinBox *positionSpinBox = new QSpinBox(this);
        positionSpinBox->setEnabled(false); //Read Only
        gridMaestro->addWidget( positionSpinBox,                r, E2_PositionSpinBox);
        //E3_Target Label ---------------------------------------
        gridMaestro->addWidget( new QLabel("Target", this),     r, E3_Target, Qt::AlignLeft );
        //E4_TargetSpinBox
        QSpinBox *targetSpinBox = new QSpinBox(this);
        targetSpinBox->setProperty("row", r); //add row property
        gridMaestro->addWidget( targetSpinBox,                  r, E4_TargetSpinBox);
        connect(targetSpinBox, &QSpinBox::editingFinished, this, [=](){slotTargetSpinBox();});
        //E5_TargetSlider----------------------------------------
        QSlider *targetSlider = new QSlider(Qt::Horizontal, this);
        targetSlider->setProperty("row", r);  //add row property
        gridMaestro->addWidget( targetSlider,                   r, E5_TargetSlider);
        connect(targetSlider, &QSlider::sliderReleased, this, [=](){slotTargetSlider();});
        //E6_Speed Label ----------------------------------------
        gridMaestro->addWidget( new QLabel("Speed", this),      r, E6_Speed, Qt::AlignLeft );
        //E7_SpeedSpinBox
        QSpinBox *speedSpinBox = new QSpinBox(this);
        speedSpinBox->setProperty("row", r); //add row property
        speedSpinBox->setMinimum(0);
        speedSpinBox->setMaximum(999);
        speedSpinBox->setValue(mMaestroSpeedInit);
        speedSpinBox->setToolTip("0~999\nHit <Enter> to apply!");

        gridMaestro->addWidget( speedSpinBox,                   r, E7_SpeedSpinBox);
        connect(speedSpinBox,  &QSpinBox::editingFinished, this, [=](){slotSpeedSpinBox();});
        //E8_Acc Label ------------------------------------------
        gridMaestro->addWidget( new QLabel("Acceleration",this),r, E8_Acc, Qt::AlignLeft  );
        //E9_AccSpinBox
        QSpinBox *accSpinBox = new QSpinBox(this);
        accSpinBox->setProperty("row", r); //add row property
        accSpinBox->setMinimum(0);
        accSpinBox->setMaximum(255);
        accSpinBox->setValue(mMaestroAccInit);
        accSpinBox->setToolTip("0~255\nHit <Enter> to apply!");
        gridMaestro->addWidget( accSpinBox,                     r, E9_AccSpinBox);
        connect(accSpinBox, &QSpinBox::editingFinished, this, [=](){slotAccSpinBox();});

        //Adjust column width
        gridMaestro->setColumnMinimumWidth(E2_PositionSpinBox,  64);
        gridMaestro->setColumnMinimumWidth(E4_TargetSpinBox,    64);
        gridMaestro->setColumnMinimumWidth(E7_SpeedSpinBox,     64);
        gridMaestro->setColumnMinimumWidth(E9_AccSpinBox,       64);
        //Adjust min/max
        positionSpinBox->setMinimum(mMaestroMinValue);
        targetSpinBox->setMinimum(mMaestroMinValue);
        targetSlider->setMinimum(mMaestroMinValue);

        positionSpinBox->setMaximum(mMaestroMaxlValue);
        targetSpinBox->setMaximum(mMaestroMaxlValue);
        targetSlider->setMaximum(mMaestroMaxlValue);

        int iStep = (mMaestroMaxlValue-mMaestroMinValue) / 50;
        positionSpinBox->setSingleStep(iStep);
        targetSpinBox->setSingleStep(iStep);
        targetSlider->setSingleStep(iStep);
        targetSlider->setTickInterval((mMaestroMaxlValue-mMaestroMinValue)/4);
        targetSlider->setTickPosition(QSlider::TicksAbove);

        int iMiddle = mMaestroMinValue+(mMaestroMaxlValue-mMaestroMinValue)/2;
        targetSlider->setValue(iMiddle);
        targetSpinBox->setValue(iMiddle);
        positionSpinBox->setValue(iMiddle);
        }
}
/*
    Maestro GoHome

    Init all TargetPosition, Speed and Acceration

 */
void MainWindow::on_pushButtonInit_clicked()
{
    //MaestroGoHome();  //reset all
    int iMiddle = mMaestroMinValue+(mMaestroMaxlValue-mMaestroMinValue)/2;
    for (int c=0;c<mMaestroChannels;c++) {
        //Sync spinbox
        QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(c, E4_TargetSpinBox);
        QWidget *widget = item->widget();
        QSpinBox *spinbox = dynamic_cast<QSpinBox*>(widget);
        spinbox->setValue(iMiddle);
        spinbox->editingFinished();

        item = ui->gridLayoutMaestro->itemAtPosition(c, E7_SpeedSpinBox);
        widget = item->widget();
        spinbox = dynamic_cast<QSpinBox*>(widget);
        spinbox->setValue(mMaestroSpeedInit);
        spinbox->editingFinished();

        item = ui->gridLayoutMaestro->itemAtPosition(c, E9_AccSpinBox);
        widget = item->widget();
        spinbox = dynamic_cast<QSpinBox*>(widget);
        spinbox->setValue(mMaestroAccInit);
        spinbox->editingFinished();
        }

}

/*
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
    MaestroSetTarget(channel, spinbox->value());
    unsigned char cLSB = (spinbox->value() & 0x7F00) >> 8;
    unsigned char cMSB = (spinbox->value() & 0x7F);
    qDebug() << "Target:" << spinbox->value() << QString::number(spinbox->value(), 16);
    qDebug() << "LSB(7bit)" << QString::number(cLSB,16) << QString::number(cLSB,2);
    qDebug() << "MSB(7bit)" << QString::number(cMSB,16) << QString::number(cMSB,2);
    int iUs = ((cLSB>>1)<<8) + cMSB;
    spinbox->setToolTip(QString::number(iUs)+"us"+"\nHit <Enter> to apply!");
    //Sync slider
    QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(row, E5_TargetSlider);
    QWidget *widget = item->widget();
    QSlider *slider = dynamic_cast<QSlider*>(widget);
    slider->setValue(spinbox->value());
    slider->setToolTip(QString::number(iUs)+"us");
    //Sync position
    int16_t pos = MaestroGetPosition(channel);
    //qDebug() << "Got pos:" << pos << QString::number(pos, 16);
    QLayoutItem* itemPos = ui->gridLayoutMaestro->itemAtPosition(row, E2_PositionSpinBox);
    QWidget *widgetPos = itemPos->widget();
    QSpinBox *spinboxPos = dynamic_cast<QSpinBox*>(widgetPos);
    spinboxPos->setValue(pos);


}
void MainWindow::slotTargetSlider()
{
    if (!bSerialOpen) return;

    QSlider *slider = static_cast<QSlider *>(QObject::sender());
    int row = slider->property("row").toInt();
    //qDebug() <<  row <<   slider->value();

    //Sync spinbox
    QLayoutItem* item = ui->gridLayoutMaestro->itemAtPosition(row, E4_TargetSpinBox);
    QWidget *widget = item->widget();
    QSpinBox *spinbox = dynamic_cast<QSpinBox*>(widget);
    spinbox->setValue(slider->value());
    spinbox->editingFinished(); //emit signal
}
void MainWindow::slotSpeedSpinBox()
{
    if (!bSerialOpen) return;

    QSpinBox *spinbox = static_cast<QSpinBox *>(QObject::sender());
    int row = spinbox->property("row").toInt();
    qDebug() <<  row <<   spinbox->value();
    char channel = static_cast<char>(row);
    MaestroSetSpeed(channel, spinbox->value());
}
void MainWindow::slotAccSpinBox()
{
    if (!bSerialOpen) return;

    QSpinBox *spinbox = static_cast<QSpinBox *>(QObject::sender());
    int row = spinbox->property("row").toInt();
    qDebug() <<  row <<   spinbox->value();
    char channel = static_cast<char>(row);
    MaestroSetAcc(channel, spinbox->value());
}


