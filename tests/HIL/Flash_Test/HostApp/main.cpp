#include <QCoreApplication>
#include <QElapsedTimer>
#include "com.h"
#include "flash_util.h"
#include <cstring>

/* Attempt to erase invalid address */
void Test1() {
    QElapsedTimer timer;
    Boot_MsgIdTypeDef msg_id = MSG_ID_ACK;
    uint8_t data[256] = {0};
    uint8_t length = 0;

    // Invalid address and size
    ToFlashPacket(0, 0, data);
    ComTransmitPacket(MSG_ID_MEM_ERASE, data, 6);
    timer.start();

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    qint64 elapsed = timer.elapsed();

    if (msg_id != MSG_ID_NACK) {
        qDebug() << "Test1 Error: Expected NACK 1, got" << msg_id;
        return;
    }

    if (elapsed > 50) {
        qDebug() << "Test1 Error: Target response too slow:" << elapsed << "ms";
        return;
    }


    // Valid address, invalid size
    msg_id = MSG_ID_ACK;
    ToFlashPacket(0x8004000, 0x200, data);
    ComTransmitPacket(MSG_ID_MEM_ERASE, data, 6);
    timer.start();

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    elapsed = timer.elapsed();

    if (msg_id != MSG_ID_NACK) {
        qDebug() << "Test1 Error: Expected NACK 2, got" << msg_id;
        return;
    }

    if (elapsed > 50) {
        qDebug() << "Test1 Error: Target response too slow:" << elapsed << "ms";
        return;
    }

    // Valid size, invalid address
    msg_id = MSG_ID_ACK;
    ToFlashPacket(0x8004400, 0x800, data);
    ComTransmitPacket(MSG_ID_MEM_ERASE, data, 6);
    timer.start();

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    elapsed = timer.elapsed();

    if (msg_id != MSG_ID_NACK) {
        qDebug() << "Test1 Error: Expected NACK 3, got" << msg_id;
        return;
    }

    if (elapsed > 50) {
        qDebug() << "Test1 Error: Target response too slow:" << elapsed << "ms";
        return;
    }

    qDebug() << "Test1: Passed" << ", response latency:" << elapsed << "ms";
}

/* Erase valid address */
void Test2() {
    QElapsedTimer timer;
    Boot_MsgIdTypeDef msg_id = MSG_ID_NACK;
    uint8_t data[256] = {0};
    uint8_t length = 0;

    ToFlashPacket(0x8004000, 0x800, data);
    ComTransmitPacket(MSG_ID_MEM_ERASE, data, 6);

    timer.start();

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 250);
    qint64 elapsed = timer.elapsed();
    if (msg_id != MSG_ID_ACK) {
        qDebug() << "Test2 Error: Expected ACK 1, got" << msg_id;
        return;
    }
    if (elapsed > 50) {
        qDebug() << "Test2 Error: Target response too slow:" << elapsed << "ms";
        return;
    }

    // Read back erased data from beginning of segment
    ToFlashPacket(0x8004000, 0x8, data);
    ComTransmitPacket(MSG_ID_MEM_READ, data, 6);

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    if (msg_id != MSG_ID_MEM_READ) {
        qDebug() << "Test2 Error: Expected MSG_ID_MEM_READ 1, got" << msg_id;
        return;
    }

    uint8_t target_data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (memcmp(data, target_data, 8) != 0) {
        qDebug() << "Test2 Error: Erase failed 1";
        return;
    }

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    if (msg_id != MSG_ID_ACK) {
        qDebug() << "Test2 Error: Expected ACK 2, got" << msg_id;
        return;
    }

    // Read back erased data from end of segment
    ToFlashPacket(0x80047F8, 0x8, data);
    ComTransmitPacket(MSG_ID_MEM_READ, data, 6);

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    if (msg_id != MSG_ID_MEM_READ) {
        qDebug() << "Test2 Error: Expected MSG_ID_MEM_READ 2, got" << msg_id;
        return;
    }

    if (memcmp(data, target_data, 8) != 0) {
        qDebug() << "Test2 Error: Erase failed 2";
        return;
    }

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    if (msg_id != MSG_ID_ACK) {
        qDebug() << "Test2 Error: Expected ACK 3, got" << msg_id;
        return;
    }

    qDebug() << "Test2: Passed" << ", erase latency:" << elapsed << "ms";
}

void Test3() {
    QElapsedTimer timer;
    Boot_MsgIdTypeDef msg_id = MSG_ID_NACK;
    uint8_t data[256] = {0};
    uint8_t length = 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ComInit();

    Test1();
    //Test2();
    Test3();

    return a.exec();
}
