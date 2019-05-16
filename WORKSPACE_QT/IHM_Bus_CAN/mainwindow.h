#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <libpcan.h>
#include <fcntl.h>    // O_RDWR
//#include "threadpcan.h"

#define ID_IHM                  0xA0
#define ID_ANEMO_PRESSURE_CARD  0xC1
#define ID_LUX_RANGE_CARD       0xC2
#define ID_IMU_CARD             0xC3


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void openCANPort();
    void receiveCANMessage();
    void refreshAnemoButton();
    void refreshPressureButton();

    void sendCANMessage(int fromId, int toId, unsigned char order, QList<int> data={0});
public slots:
    void updateCanData();
    void onRefreshAnemoButton();
    void onRefreshPressureButton();
    void onAutoRefreshButton(bool state);
    void onLuminositeButton();
    void onDistanceButton();
    //void onLux(bool state);

    void onTimer_Tick();

private:
    Ui::MainWindow *ui;

    HANDLE h;
    TPCANRdMsg pMsgBuff;
    QTimer *timer_tick;
    QTimer *timer2_tick;

    float pressure;
    float temperature;
    float windSpeed;
    int distance;
    int lux;
    bool luxSwitchState;
    bool distSwitchState;
    int updateState;
    //ThreadPCAN *thread;
};

#endif // MAINWINDOW_H
