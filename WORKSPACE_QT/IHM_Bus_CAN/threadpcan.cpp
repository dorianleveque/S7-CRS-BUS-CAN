#include "threadPcan.h"

ThreadPCAN::ThreadPCAN(QObject *parent) : QThread(parent)
{
    openCANPort();
}

void ThreadPCAN::run()
{
    while(!this->stopRequest) {
        LINUX_CAN_Read_Timeout(h, &pMsgBuff, -1);
        int toid = int(pMsgBuff.Msg.ID);
        int lenght = int(pMsgBuff.Msg.LEN);
        int fromId = int(pMsgBuff.Msg.DATA[0]);
        char data_type = char(pMsgBuff.Msg.DATA[1]);
        QList<int> data_tmp;
        //printf("ID: %d lenght: %d\n", id, lenght);
        emit newPcanMessage(fromId, data_type, data_tmp, lenght);
    }
    emit terminate();
}

void ThreadPCAN::openCANPort()
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

void ThreadPCAN::sendCANMessage(int fromId, int toId, unsigned char order, QList<int> data)
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
}

// slots
void ThreadPCAN::stop()
{
    this->stopRequest = true;
    this->quit();
}
