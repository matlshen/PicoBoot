#include <QCoreApplication>
#include "boot.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ComInit();

    Boot_StatusTypeDef status;
    Boot_MsgIdTypeDef msgId;
    uint8_t length;
    uint8_t data[5] = {0x01, 0x02, 0x03, 0x04, 0x05};


    while (1) {
        if (ComReceivePacket(&msgId, data, &length, 10000) == BOOT_OK) {
            qDebug() << "Received packet";
        } else {
            qDebug() << "Receive failed";
        }
        ComTransmitPacket(msgId, data, length);
    }


    return a.exec();
}
