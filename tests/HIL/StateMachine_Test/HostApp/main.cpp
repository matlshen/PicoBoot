#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>
#include <QDebug>
#include <QRandomGenerator>
#include "com.h"
#include "flash_util.h"
#include "host.h"
#include <cstring>

void InitiateConnection() {
    // Send empty connection request packet
    ComTransmitPacket(MSG_ID_CONN_REQ, NULL, 0);
    // Don't check response, target may already be connected
    QThread::msleep(10);
}

int WaitForPacket(Boot_MsgIdTypeDef expected_id, int timeout, QString test_name) {
    QElapsedTimer timer;
    Boot_MsgIdTypeDef msg_id = MSG_ID_ACK;
    uint8_t data[8] = {0};
    uint8_t length = 0;

    timer.start();

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, (uint32_t)timeout);
    qint64 elapsed = timer.elapsed();
    if (elapsed > timeout) {
        qDebug() << test_name << "Error: Target timed out after" << elapsed << "ms";
        return -1;
    }
    if (msg_id != expected_id) {
        qDebug() << test_name << "Error:" << "Unexpected packet, got:" << msg_id;
        return -1;
    }

    return elapsed;
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    return a.exec();
}
