#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
//#include <fcntl.h>    // O_RDWR
//#include <objectgl.h>
#include "threadPcan.h"

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

public slots:
    void updateCanData(int value);

    void onReceiveCANMessage(int fromid, char data_type, QList<int> data_tmp, int len);
    void onRefreshAnemoButton();
    void onRefreshPressureButton();
    void onAutoRefreshButton(bool state);
    void onLuminositeButton();
    void onDistanceButton();
    //void onLux(bool state);

    //void onTimer_Tick();

protected slots:
    // Redraw the scene
    //void onTimer_UpdateDisplay();

protected:
    // Overload of the resize event
    //void resizeEvent(QResizeEvent *);

private:
    Ui::MainWindow *ui;
    //TPCANRdMsg pMsgBuff;
    ThreadPCAN *th_receiver;

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
};

#endif // MAINWINDOW_H
