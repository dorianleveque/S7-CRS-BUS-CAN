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
    //connect(timer2_tick, SIGNAL(timeout()), this, SLOT(onRefreshPressureButton()));

    timer_tick -> start(1); // 1ms
    timer2_tick -> start(50);

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
    sendCANMessage(ID_ANEMO_PRESSURE_CARD, {'P'});
}

void MainWindow::refreshAnemoButton()
{
    sendCANMessage(ID_ANEMO_PRESSURE_CARD, {'P', 'T'});
}

void MainWindow::sendCANMessage(int id, QList<int> data)
{
    TPCANMsg msgBuff;

    msgBuff.ID      =   static_cast<unsigned char>(id);
    msgBuff.MSGTYPE =   MSGTYPE_STANDARD;
    msgBuff.LEN     =   static_cast<unsigned char>(data.length());

    for(int i=0; i<data.length(); i++) {
        msgBuff.DATA[i] = static_cast<unsigned char> (data[i]);
    }

    //LINUX_CAN_Write_Timeout(h, &msgBuff,0);
    //CAN_Write(h, &msgBuff);
}


void MainWindow::receiveCANMessage()
{
    LINUX_CAN_Read_Timeout(h, &pMsgBuff, -1);
    //LINUX_CAN_Read_Timeout(h, &pMsgBuff, 0);
    int id = int(pMsgBuff.Msg.ID);
    int lenght = int(pMsgBuff.Msg.LEN);
    //printf("ID: %d lenght: %d\n", id, lenght);
    unsigned int data_type;

    switch (id) {
    case ID_ANEMO_PRESSURE_CARD:
        data_type = (unsigned int) pMsgBuff.Msg.DATA[0];
        switch (data_type) {
        case 'P':
            for(int i=1; i<lenght; i++) {
                pressure = pMsgBuff.Msg.DATA[i] << (32-8*i);
            }
            ui -> pressureField -> setText(QString::number(pressure));
            break;
        case 'T':
            for(int i=1; i<lenght; i++) {
                temperature = pMsgBuff.Msg.DATA[i] << (32-8*i);
            }
            break;
        case 'W':
            for(int i=1; i<lenght; i++) {
                windSpeed = pMsgBuff.Msg.DATA[i] << (32-8*i);
            }
            ui -> anemoField -> setText(QString::number(windSpeed));
            break;
        }
        break;
    case ID_LUX_RANGE_CARD:
        switch (data_type) {
        case 'L':
            for(int i=1; i<lenght; i++) {
                lux = pMsgBuff.Msg.DATA[i] << (32-8*i);
            }
            ui->lumDistanceField-> setText(QString::number(lux));
            break;
        case 'D':
            for(int i=1; i<lenght; i++) {
                distance = pMsgBuff.Msg.DATA[i] << (32-8*i);
            }
            break;
        }
        break;

            /*float pressure;
            unsigned int a0, a1, a2, a3;
            a0 = (unsigned int) pMsgBuff.Msg.DATA[0];
            a1 = (unsigned int) pMsgBuff.Msg.DATA[1];
            a2 = (unsigned int) pMsgBuff.Msg.DATA[2];
            a3 = (unsigned int) pMsgBuff.Msg.DATA[3];
            pressure = (float) (a0<<24 | a1<<16 | a2<<8 | a3);

            /*printf("a0 %x\n", pMsgBuff.Msg.DATA[0]);
            printf("a1 %x\n", pMsgBuff.Msg.DATA[1]);
            printf("a2 %x\n", pMsgBuff.Msg.DATA[2]);
            printf("a3 %x\n", pMsgBuff.Msg.DATA[3]);*/
            printf("pressure %f\n\n", pressure);
            /*for(int i=0; i<lenght; i++) {
                pressure = pMsgBuff.Msg.DATA[i] << (32-8*(i+1));
                printf("data[%d]= %x\n", i, pMsgBuff.Msg.DATA[i]);
            }
            ui -> pressureField -> setText(QString::number(pressure));*/

        /*case 2:
            long unsigned int speed;
            for(int i=0; i<pMsgBuff.Msg.LEN; i++) {
                speed |= pMsgBuff.Msg.DATA[i] << (32-8*(i+1));
            }
            ui -> anemoField -> setText(QString::number(speed));
        break;*/
    }
}


/////////////////
// SLOT
void MainWindow::onRefreshAnemoButton()
{
    refreshAnemoButton();
}

void MainWindow::onRefreshPressureButton()
{
    refreshPressureButton();
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
