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

    refresh_timer = new QTimer();
    refresh_timer->start(10); // 10ms
    connect(refresh_timer, SIGNAL(timeout()), this, SLOT(onRefreshCanData()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onReceiveCANMessage(int fromId, char data_type, QList<int> data, int len)
{
    printf("ID: %x lenght: %d\n", fromId, len);
    int tmp_data;
    switch (fromId) {
    case ID_ANEMO_PRESSURE_CARD:
        switch (data_type) {
        case 'P':
            tmp_data = data[0]<<24 | data[1]<<16 | data[2]<<8 | data[3];
            ui -> pressureField -> setText(QString::number((float)tmp_data));
            break;
        case 'T':
            tmp_data = data[0]<<24 | data[1]<<16 | data[2]<<8 | data[3];
            ui -> temperatureField -> setText(QString::number((float)tmp_data));
            break;
        case 'W':
            tmp_data = data[0]<<24 | data[1]<<16 | data[2]<<8 | data[3];
            ui -> anemoField -> setText(QString::number((float)tmp_data));
            break;
        }
        break;
    case ID_LUX_RANGE_CARD:
        switch (data_type) {
        case 'L':
            tmp_data = data[0]<<24 | data[1]<<16 | data[2]<<8 | data[3];
            lux = (float) tmp_data;
            luxSelectState = true;
            break;
        case 'R':
            tmp_data = data[0]<<24 | data[1]<<16 | data[2]<<8 | data[3];
            range = (float) tmp_data;
            luxSelectState = false;
            break;
        }
        if (luxSelectState) {
            ui->luxRangeField-> setText(QString::number(lux));
            ui->luxRangeLabel->setText("lux");
        }
        else{
            ui->luxRangeField-> setText(QString::number(range));
            ui->luxRangeLabel->setText("mm");
        }
        break;
    }
}


/////////////////
// SLOT

void MainWindow::onRefreshCanData() {
   switch(updateState)
    {
        case 0: th_receiver->sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, 'P'); break;
        case 1: th_receiver->sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, 'T'); break;
        case 2: th_receiver->sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, 'W'); break;
        case 3: th_receiver->sendCANMessage(ID_IHM, ID_LUX_RANGE_CARD, 'D'); break;
        case 4: th_receiver->sendCANMessage(ID_IHM, ID_LUX_RANGE_CARD, 'A'); break;
        default: updateState=0;
    }
    updateState++;
}

void MainWindow::onRefreshAnemoButton()
{
    th_receiver->sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, 'W');
}

void MainWindow::onRefreshPressureButton()
{
    th_receiver->sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, 'P');
}

void MainWindow::onLuxButton()
{
    luxSelectState = true;
    th_receiver->sendCANMessage(ID_IHM, ID_LUX_RANGE_CARD, 'X', { 0x01 });
}

void MainWindow::onRangeButton()
{
    luxSelectState = false;
    th_receiver->sendCANMessage(ID_IHM, ID_LUX_RANGE_CARD, 'X', { 0x00 });
}
