#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
//#include <fcntl.h>    // O_RDWR
//#include <objectgl.h>
#include "threadpcan.h"

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
    void onRefreshCanData();
    void onReceiveCANMessage(int fromid, char data_type, QList<int> data_tmp, int len);
    void onRefreshAnemoButton();
    void onRefreshPressureButton();
    void onLuxButton();
    void onRangeButton();

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

    QTimer *refresh_timer;
    QTimer *timer2_tick;

    float pressure;
    float temperature;
    float windSpeed;
    float range;
    float lux;
    bool luxSelectState;
    int updateState;
};

#endif // MAINWINDOW_H
