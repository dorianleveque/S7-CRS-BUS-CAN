/*#ifndef THREADPCAN_H
#define THREADPCAN_H

#include <QThread>
#include <libpcan.h>

class ThreadPCAN : public QThread
{
    Q_OBJECT
public:
    explicit ThreadPCAN(QObject *parent = nullptr);

signals:
    void valueChanged(int);
    void finished();

public slots:
    void run() override;
};

#endif // THREADPCAN_H
*/
