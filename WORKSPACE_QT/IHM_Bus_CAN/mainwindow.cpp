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
    connect(th_receiver, SIGNAL(terminate()), this, SLOT(onThreadPcanTerminate()));

    refresh_timer = new QTimer();
    refresh_timer->start(10); // 10ms
    connect(refresh_timer, SIGNAL(timeout()), this, SLOT(onRefreshCanData()));

    // Create the openGL display for the map
    Object_GL = new ObjectOpenGL();
    Object_GL->setObjectName(QString::fromUtf8("ObjectOpenGL"));
    Object_GL->setGeometry(QRect(0, 0, this->width(), this->height()));
    ui->IMU_layout->addWidget(Object_GL, 0, 0, 1, 1);


    timerDisplay = new QTimer();
    timerDisplay->connect(timerDisplay, SIGNAL(timeout()),this, SLOT(onTimer_UpdateDisplay()));
    timerDisplay->start(50);
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
            ui -> pressureField -> display((float)tmp_data);
            break;
        case 'T':
            tmp_data = data[0]<<24 | data[1]<<16 | data[2]<<8 | data[3];
            ui -> temperatureField -> display((float)tmp_data);
            break;
        case 'W':
            tmp_data = data[0]<<24 | data[1]<<16 | data[2]<<8 | data[3];
            ui -> anemoField -> display((float)tmp_data);
            break;
        }
        break;
    case ID_LUX_RANGE_CARD:
        switch (data_type) {
        case 'L':
            tmp_data = data[0]<<24 | data[1]<<16 | data[2]<<8 | data[3];
            lux = (float) tmp_data;
            luxSelectState = true;
            ui -> lux_button -> setChecked(true);
            break;
        case 'R':
            tmp_data = data[0]<<24 | data[1]<<16 | data[2]<<8 | data[3];
            range = (float) tmp_data;
            luxSelectState = false;
            ui -> range_button -> setChecked(true);
            break;
        }
        if (luxSelectState) {
            ui->luxRangeField-> display(lux);
            ui->luxRangeLabel->setText("lux");
        }
        else{
            ui->luxRangeField-> display(range);
            ui->luxRangeLabel->setText("mm");
        }
        break;
    case ID_IMU_CARD:
        if (data_type == 'A')
        {
            phiAngle = 180+ (data[0]<<8 | data[1]);
            psiAngle = 180+ (data[2]<<8 | data[3]);
            tetaAngle = 180+ (data[4]<<8 | data[5]);
            Object_GL->setAngles(phiAngle, psiAngle, tetaAngle);
        }
    }
}


/////////////////
// SLOT

void MainWindow::onRefreshCanData() {
   switch(updateState)
    {
        case 1: th_receiver->sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, 'P'); break;
        case 2: th_receiver->sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, 'T'); break;
        case 3: th_receiver->sendCANMessage(ID_IHM, ID_ANEMO_PRESSURE_CARD, 'W'); break;
        case 4: th_receiver->sendCANMessage(ID_IHM, ID_LUX_RANGE_CARD, 'D'); break;
        case 5: th_receiver->sendCANMessage(ID_IHM, ID_IMU_CARD, 'A'); break;
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
    ui->luxRangeLabel->setText("lux");
    th_receiver->sendCANMessage(ID_IHM, ID_LUX_RANGE_CARD, 'X', { 1 });
}

void MainWindow::onRangeButton()
{
    luxSelectState = false;
    ui->luxRangeLabel->setText("mm");
    th_receiver->sendCANMessage(ID_IHM, ID_LUX_RANGE_CARD, 'X', { 0 });
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    Object_GL->resize(this->width(),this->height());
    //gridLayoutWidget->setGeometry(QRect(0, 0, centralWidget->width(), centralWidget->height()));
}

void MainWindow::onTimer_UpdateDisplay()
{
    Object_GL->updateGL();
}

void MainWindow::onThreadPcanTerminate()
{
    printf("thread pcan stoped");
}
