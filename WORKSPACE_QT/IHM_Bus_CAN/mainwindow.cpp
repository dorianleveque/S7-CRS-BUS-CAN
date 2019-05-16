#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

#define DEFAULT_NODE "/dev/pcanusb32"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("BusCAN");

    openCANPort();

    timer_tick = new QTimer();
    timer2_tick= new QTimer();
    connect(timer_tick, SIGNAL(timeout()), this, SLOT(onTimer_Tick()));
    connect(timer2_tick, SIGNAL(timeout()), this, SLOT(updateCanData()));

    timer_tick -> start(10); // 1ms
    //timer2_tick -> start(10);

    /*thread = new threadPCAN(this);
    thread.run();*/
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::openCANPort()
{
    const char *szDevNode = DEFAULT_NODE;

    h=LINUX_CAN_Open(szDevNode, O_RDWR);
    if (!h)
        {
            printf("can't open %s\n", szDevNode);
        }
    CAN_Init(h, CAN_BAUD_500K,  CAN_INIT_TYPE_ST);         // BTR0BTR1	bitrate code in hex (default=0x1c (500 kbps))
    CAN_Status(h); // Clear Status
}

void MainWindow::refreshPressureButton()
{
    sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, {'P'});
}

void MainWindow::refreshAnemoButton()
{
    sendCANMessage(ID_IHM, ID_LUX_RANGE_CARD, {'W'});
}

void MainWindow::sendCANMessage(int fromId, int toId, unsigned char order, QList<int> data)
{
    TPCANMsg msgBuff;

    msgBuff.ID      =   static_cast<unsigned char>(toId);
    msgBuff.MSGTYPE =   MSGTYPE_STANDARD;
    msgBuff.LEN     =   static_cast<unsigned char>(data.length()+1);

    msgBuff.DATA[0] = static_cast<unsigned char>(fromId);
    msgBuff.DATA[1] = order;
    for(int i=0; i<data.length(); i++) {
        msgBuff.DATA[i+2] = static_cast<unsigned char> (data[i]);
    }

    LINUX_CAN_Write_Timeout(h, &msgBuff,0);
    //CAN_Write(h, &msgBuff);
}


void MainWindow::receiveCANMessage()
{
    LINUX_CAN_Read_Timeout(h, &pMsgBuff, -1);
    //LINUX_CAN_Read_Timeout(h, &pMsgBuff, 0);
    int fromId = int(pMsgBuff.Msg.DATA[0]);
    char data_type = char(pMsgBuff.Msg.DATA[1]);
    int lenght = int(pMsgBuff.Msg.LEN);
    float data_tmp = float (pMsgBuff.Msg.DATA[2]<<24 | pMsgBuff.Msg.DATA[3]<<16 | pMsgBuff.Msg.DATA[4]<<8 | pMsgBuff.Msg.DATA[5]);
    //printf("ID: %x lenght: %d\n", fromId, lenght);
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
    }
}


/////////////////
// SLOT

void MainWindow::updateCanData() {
   /* switch(updateState)
    {
        case 0: sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, {'P'}); break;
        case 1: sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, {'T'}); break;
        case 2: sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, {'W'}); break;
        case 3: sendCANMessage(ID_IHM, ID_LUX_RANGE_CARD, {'L'}); break;
        default: updateState=0;
    }
    updateState++;*/
}

void MainWindow::onRefreshAnemoButton()
{
    refreshAnemoButton();
}

void MainWindow::onRefreshPressureButton()
{
    refreshPressureButton();
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

void MainWindow::onTimer_Tick()
{
    receiveCANMessage();
}
