/*#include "threadpcan.h"

ThreadPCAN::ThreadPCAN(QObject *parent) : QThread(parent)
{

}

ThreadPCAN::run() override
{
    while(1) {
        LINUX_CAN_Read_Timeout(h, &pMsgBuff, -1);
        int id = int(pMsgBuff.Msg.ID);
        int lenght = int(pMsgBuff.Msg.LEN);
        printf("ID: %d lenght: %d\n", id, lenght);
        emit valueChanged(id);
    }
    emit finished();
}
*/
