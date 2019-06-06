#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("BusCAN");

    th_receiver = new ThreadPCAN();
    th_receiver -> start();
    connect(th_receiver, SIGNAL(newPcanMessage(int, char, QList<int>, int)), this, SLOT(onReceiveCANMessage(int, char, QList<int>, int)));

    //timer_tick = new QTimer();
    timer2_tick= new QTimer();
    //connect(timer2_tick, SIGNAL(timeout()), this, SLOT(updateCanData()));

    //timer_tick -> start(10); // 1ms
    //timer2_tick -> start(10);

    //connect(receivePcanThread, SIGNAL(valueChanged()), this, SLOT(updateCanData()));
    //receivePcanThread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onReceiveCANMessage(int fromid, char data_type, QList<int> data_tmp, int len)
{
    //LINUX_CAN_Read_Timeout(h, &pMsgBuff, -1);
    //LINUX_CAN_Read_Timeout(h, &pMsgBuff, 100);
    /*int fromId = int(pMsgBuff.Msg.DATA[0]);
    char data_type = char(pMsgBuff.Msg.DATA[1]);
    int lenght = int(pMsgBuff.Msg.LEN);
    float data_tmp = float (pMsgBuff.Msg.DATA[2]<<24 | pMsgBuff.Msg.DATA[3]<<16 | pMsgBuff.Msg.DATA[4]<<8 | pMsgBuff.Msg.DATA[5]);
    printf("ID: %x lenght: %d\n", fromId, lenght);
    //unsigned int data_type;

    switch (fromId) {
    case ID_ANEMO_PRESSURE_CARD:
        switch (data_type) {
        case 'P':
            pressure = data_tmp;
            ui -> pressureField -> setText(QString::number(pressure));
            break;
        case 'T':
            temperature = data_tmp;
            ui -> temperatureField -> setText(QString::number(temperature));
            break;
        case 'W':
            windSpeed = data_tmp;
            ui -> anemoField -> setText(QString::number(windSpeed));
            break;
        }
        break;
    case ID_LUX_RANGE_CARD:
        switch (data_type) {
        case 'L':
            lux = data_tmp;
            ui->lumDistanceField-> setText(QString::number(lux));
            break;
        case 'D':
            distance = data_tmp;
            break;
        }
        break;
    }*/
}


/////////////////
// SLOT

void MainWindow::updateCanData(int value) {
   /* switch(updateState)
    {
        case 0: sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, {'P'}); break;
        case 1: sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, {'T'}); break;
        case 2: sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, {'W'}); break;
        case 3: sendCANMessage(ID_IHM, ID_LUX_RANGE_CARD, {'L'}); break;
        default: updateState=0;
    }
    updateState++;*/
    printf("%d \n", value);
}

void MainWindow::onRefreshAnemoButton()
{
    th_receiver->sendCANMessage(ID_IHM, ID_LUX_RANGE_CARD, 'X', { 0x00, 0x32 });
}

void MainWindow::onRefreshPressureButton()
{
    th_receiver->sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, {'P'});
}

void MainWindow::onLuminositeButton()
{
    luxSwitchState = true;
    distSwitchState = false;
}

void MainWindow::onDistanceButton()
{
    luxSwitchState = false;
    distSwitchState = true;
}

void MainWindow::onAutoRefreshButton(bool state)
{
    if (state) {
        timer_tick -> start(10); // 1ms
    }
    else {
        timer_tick ->stop();
    }
}
