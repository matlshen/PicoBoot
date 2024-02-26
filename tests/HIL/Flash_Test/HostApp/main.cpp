#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>
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

/* Write invalid address */
void Test3() {
    QElapsedTimer timer;
    Boot_MsgIdTypeDef msg_id = MSG_ID_ACK;
    uint8_t data[256] = {0};
    uint8_t length = 0;

    // Invalid address and size
    ToFlashPacket(0, 0, data);
    ComTransmitPacket(MSG_ID_MEM_WRITE, data, 6);
    timer.start();

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    qint64 elapsed = timer.elapsed();

    if (msg_id != MSG_ID_NACK) {
        qDebug() << "Test3 Error: Expected NACK 1, got" << msg_id;
        return;
    }

    if (elapsed > 50) {
        qDebug() << "Test3 Error: Target response too slow:" << elapsed << "ms";
        return;
    }

    // Valid address, invalid size
    msg_id = MSG_ID_ACK;
    ToFlashPacket(0x8004000, 0x1, data);
    ComTransmitPacket(MSG_ID_MEM_WRITE, data, 6);
    timer.start();

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    elapsed = timer.elapsed();

    if (msg_id != MSG_ID_NACK) {
        qDebug() << "Test3 Error: Expected NACK 2, got" << msg_id;
        return;
    }

    if (elapsed > 50) {
        qDebug() << "Test3 Error: Target response too slow:" << elapsed << "ms";
        return;
    }

    // Valid size, invalid address
    msg_id = MSG_ID_ACK;
    ToFlashPacket(0x8003000, 0x800, data);
    ComTransmitPacket(MSG_ID_MEM_WRITE, data, 6);
    timer.start();

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    elapsed = timer.elapsed();

    if (msg_id != MSG_ID_NACK) {
        qDebug() << "Test3 Error: Expected NACK 3, got" << msg_id;
        return;
    }

    if (elapsed > 50) {
        qDebug() << "Test3 Error: Target response too slow:" << elapsed << "ms";
        return;
    }

    qDebug() << "Test3: Passed" << ", response latency:" << elapsed << "ms";
}

/* Perform valid write */
void Test4() {
    QElapsedTimer timer;
    Boot_MsgIdTypeDef msg_id = MSG_ID_NACK;
    uint8_t data[256] = {0};
    uint8_t length = 0;

    // Send mem erase request
    ToFlashPacket(0x8005000, 0x800, data);
    ComTransmitPacket(MSG_ID_MEM_ERASE, data, 6);

    // Wait for ACK
    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    if (msg_id != MSG_ID_ACK) {
        qDebug() << "Test4 Error: Expected ACK 1, got" << msg_id;
        return;
    }

    // Attempt to write data
    // Send mem write request
    ToFlashPacket(0x8005000, 0x8, data);
    ComTransmitPacket(MSG_ID_MEM_WRITE, data, 6);

    // Wait for ACK
    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    if (msg_id != MSG_ID_ACK) {
        qDebug() << "Test4 Error: Expected ACK 2, got" << msg_id;
        return;
    }


    // Send write data
    uint8_t write_data[8] = {0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B};
    ComTransmitPacket(MSG_ID_MEM_WRITE, write_data, 8);
    timer.start();

    // Wait for ACK
    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    qint64 elapsed = timer.elapsed();
    if (msg_id != MSG_ID_ACK) {
        qDebug() << "Test4 Error: Expected ACK 3, got" << msg_id;
        return;
    }
    if (elapsed > 50) {
        qDebug() << "Test4 Error: Target response too slow:" << elapsed << "ms";
        return;
    }

    // Read back written data
    ToFlashPacket(0x8005000, 0x8, data);
    ComTransmitPacket(MSG_ID_MEM_READ, data, 6);

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    if (msg_id != MSG_ID_MEM_READ) {
        qDebug() << "Test4 Error: Expected MSG_ID_MEM_READ 1, got" << msg_id;
        return;
    }
    if (memcmp(data, write_data, 8) != 0) {
        qDebug() << "Test4 Error: Write failed";
        return;
    }

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    if (msg_id != MSG_ID_ACK) {
        qDebug() << "Test4 Error: Expected ACK 4, got" << msg_id;
        return;
    }

    qDebug() << "Test4: Passed" << ", write latency:" << elapsed << "ms";
}

/* Write large section of data */
void Test5() {
    QElapsedTimer timer;
    Boot_MsgIdTypeDef msg_id = MSG_ID_NACK;
    uint8_t data[256] = {0};
    uint8_t length = 0;

    // Send mem erase request
    ToFlashPacket(0x8005000, 0x800, data);
    ComTransmitPacket(MSG_ID_MEM_ERASE, data, 6);

    // Wait for ACK
    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    if (msg_id != MSG_ID_ACK) {
        qDebug() << "Test5 Error: Expected ACK 1, got" << msg_id;
        return;
    }

    // Attempt to write data
    // Send mem write request
    ToFlashPacket(0x8005000, 0x200, data);
    ComTransmitPacket(MSG_ID_MEM_WRITE, data, 6);

    // Wait for ACK
    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    if (msg_id != MSG_ID_ACK) {
        qDebug() << "Test5 Error: Expected ACK 2, got" << msg_id;
        return;
    }

    // Send write data
    uint8_t write_data[512] = {0};
    for (int i = 0; i < 512; i++) {
        write_data[i] = i % 256;
    }
    for (int i = 0; i < 64; i++) {
        ComTransmitPacket(MSG_ID_MEM_WRITE, write_data, 8);
        QThread::msleep(30);
    }
    timer.start();

    // Wait for ACK
    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    qint64 elapsed = timer.elapsed();
    if (msg_id != MSG_ID_ACK) {
        qDebug() << "Test5 Error: Expected ACK 3, got" << msg_id;
        return;
    }
    if (elapsed > 50) {
        qDebug() << "Test5 Error: Target response too slow:" << elapsed << "ms";
        return;
    }

    // Read back written data
    ToFlashPacket(0x8005000, 0x1000, data);
    ComTransmitPacket(MSG_ID_MEM_READ, data, 6);

    for (int i = 0; i < 64; i++) {
        ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
        if (msg_id != MSG_ID_MEM_READ) {
            qDebug() << "Test5 Error: Expected MSG_ID_MEM_READ 1, got" << msg_id;
            return;
        }
        if (memcmp(data, write_data + i * 8, 8) != 0) {
            qDebug() << "Test5 Error: Write failed";
            return;
        }
    }

    qDebug() << "Test5: Passed" << ", write latency:" << elapsed << "ms";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ComInit();

    //Test1();
    //Test2();
    //Test3();
    //Test4();
    Test5();

    return a.exec();
}
