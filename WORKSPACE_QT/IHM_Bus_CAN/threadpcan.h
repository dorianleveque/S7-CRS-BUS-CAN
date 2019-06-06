#ifndef THREAD_PCAN_H
#define THREAD_PCAN_H
#include <QThread>
#include <QMetaType>
#include <libpcan.h>
#include <fcntl.h>    // O_RDWR

#define DEFAULT_NODE "/dev/pcanusb32"

class ThreadPCAN : public QThread
{
    Q_OBJECT

public:
    ThreadPCAN(QObject *parent=0);
    void openCANPort();
    void sendCANMessage(int fromId, int toId, unsigned char order, QList<int> data={0});

protected:
    void run();

signals:
    void newPcanMessage(int fromid, char data_type, QList<int> data, int len);
    void terminate();

public slots:
    void stop();

private:
    HANDLE h;
    TPCANRdMsg pMsgBuff;
    bool stopRequest =false;


};

#endif // THREAD_PCAN_H

