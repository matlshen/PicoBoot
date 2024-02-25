#include <QCoreApplication>
#include <QElapsedTimer>
#include "com.h"

/* Wait for consecutive NACKs to be received */
void Test1() {
    QElapsedTimer timer;
    Boot_MsgIdTypeDef msg_id = MSG_ID_NACK;
    uint8_t data[256] = {0};
    uint8_t length = 0;

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    timer.start();

    if (msg_id != MSG_ID_ACK) {
        qDebug() << "Test1 Error: Expected NACK 1, got" << msg_id;
        return;
    }

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    qint64 elapsed = timer.elapsed();

    if (msg_id != MSG_ID_ACK) {
        qDebug() << "Test1 Error: Expected NACK 2, got" << msg_id;
        return;
    }

    if (elapsed < 50*0.9) {
        qDebug() << "Test1 Error: Target timeout too short, " << elapsed << "ms";
        return;
    }

    if (elapsed > 50*1.1) {
        qDebug() << "Test1 Error: Target timeout too long, " << elapsed << "ms";
        return;
    }

    qDebug() << "Test1: Passed";
}

/* Echo packet */
void Test2() {
    QElapsedTimer timer;
    Boot_MsgIdTypeDef msg_id = MSG_ID_NACK;
    uint8_t data[256] = {0};
    uint8_t length = 0;

    ComTransmitPacket(MSG_ID_VERIFY, (uint8_t*)"testing", 8);
    timer.start();

    ComReceivePacket(&msg_id, data, &length, 100);
    qint64 elapsed = timer.elapsed();

    if (msg_id != MSG_ID_VERIFY) {
        qDebug() << "Test2 Error: Expected VERIFY, got" << msg_id;
        return;
    }

    if (length != 8) {
        qDebug() << "Test2 Error: Expected length 8, got" << length;
        return;
    }

    if (memcmp(data, "testing", 8) != 0) {
        qDebug() << "Test2 Error: Expected data 'testing', got" << data;
        return;
    }

    qDebug() << "Test2: Passed" << ", Latency:" << elapsed << "ms";
}

/* Transmit incomplete packet */
void Test3() {
    QElapsedTimer timer;
    Boot_MsgIdTypeDef msg_id = MSG_ID_VERIFY;
    uint8_t data[256] = {0};
    uint8_t length = 20;


    // Transmit the message ID
    ComTransmit((uint8_t*)&msg_id, 1, 100);

    // Transmit the length (too long)
    ComTransmit(&length, 1, 100);

    // Transmit the data (shorter than length)
    ComTransmit((uint8_t*)"testing", length / 2, 100);
    timer.start();

    ComReceivePacket(&msg_id, data, &length, 1000);
    qint64 elapsed = timer.elapsed();

    if (msg_id != MSG_ID_NACK) {
        qDebug() << "Test3 Error: Expected NACK, got" << msg_id;
        return;
    }

    if (length != 0) {
        qDebug() << "Test3 Error: Expected length 0, got" << length;
        return;
    }

    if (elapsed < ComGetTimeoutMs()*0.9) {
        qDebug() << "Test3 Error: Target timeout too short, " << elapsed << "ms";
        return;
    }

    if (elapsed > ComGetTimeoutMs()*1.1) {
        qDebug() << "Test3 Error: Target timeout too long, " << elapsed << "ms";
        return;
    }

    qDebug() << "Test3: Passed" << ", Latency:" << elapsed << "ms";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ComInit();

    Test1();
    Test2();
    Test3();

    return a.exec();
}
