#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>
#include "com.h"
#include "flash_util.h"
#include "crc32.h"
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

/* Attempt to erase invalid address */
void Test1() {
    Boot_MsgIdTypeDef msg_id = MSG_ID_ACK;
    uint8_t data[256] = {0};
    uint8_t length = 0;

    // Invalid address and size
    ToFlashPacket(0, 0, data);
    ComTransmitPacket(MSG_ID_MEM_ERASE, data, 6);
    // Wait for NACK
    if (WaitForPacket(MSG_ID_NACK, 10, "Test1") == -1) {
        return;
    }

    // Valid address, invalid size
    msg_id = MSG_ID_ACK;
    ToFlashPacket(0x8004000, 0x200, data);
    ComTransmitPacket(MSG_ID_MEM_ERASE, data, 6);
    // Wait for NACK
    if (WaitForPacket(MSG_ID_NACK, 10, "Test1") == -1) {
        return;
    }


    // Valid size, invalid address
    msg_id = MSG_ID_ACK;
    ToFlashPacket(0x8004400, 0x800, data);
    ComTransmitPacket(MSG_ID_MEM_ERASE, data, 6);
    // Wait for NACK
    qint64 elapsed = WaitForPacket(MSG_ID_NACK, 10, "Test1");
    if (elapsed == -1) {
        return;
    }

    qDebug() << "Test1: Passed" << ", response latency:" << elapsed << "ms";
}

/* Erase valid address */
void Test2() {
    Boot_MsgIdTypeDef msg_id = MSG_ID_NACK;
    uint8_t data[256] = {0};
    uint8_t length = 0;

    // Erase valid address
    ToFlashPacket(0x8004000, 0x800, data);
    ComTransmitPacket(MSG_ID_MEM_ERASE, data, 6);
    // Wait for ACK
    qint64 elapsed = WaitForPacket(MSG_ID_ACK, 100, "Test2");
    if (elapsed == -1)
        return;

    // Read back erased data from beginning of segment
    ToFlashPacket(0x8004000, 0x8, data);
    ComTransmitPacket(MSG_ID_MEM_READ, data, 6);
    // Wait for ACK
    if (WaitForPacket(MSG_ID_ACK, 100, "Test2") == -1) {
        return;
    }
    // Wait for read response
    ComReceive(data, 0x8, 100);
    uint8_t target_data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (memcmp(data, target_data, 8) != 0) {
        qDebug() << "Test2 Error: Erase failed 1";
        return;
    }
    // Wait for ACK
    if (WaitForPacket(MSG_ID_ACK, 10, "Test2") == -1) {
        return;
    }


    // Read back erased data from beginning of segment
    ToFlashPacket(0x80047F8, 0x8, data);
    ComTransmitPacket(MSG_ID_MEM_READ, data, 6);
    // Wait for ACK
    if (WaitForPacket(MSG_ID_ACK, 100, "Test2") == -1) {
        return;
    }
    // Wait for read response
    ComReceive(data, 0x8, 100);
    if (memcmp(data, target_data, 8) != 0) {
        qDebug() << "Test2 Error: Erase failed 1";
        return;
    }
    // Wait for ACK
    if (WaitForPacket(MSG_ID_ACK, 10, "Test2") == -1) {
        return;
    }

    qDebug() << "Test2: Passed";
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
    // Wait for NACK
    if (WaitForPacket(MSG_ID_NACK, 10, "Test3") == -1) {
        return;
    }

    // Valid address, invalid size
    msg_id = MSG_ID_ACK;
    ToFlashPacket(0x8004000, 0x1, data);
    ComTransmitPacket(MSG_ID_MEM_WRITE, data, 6);
    // Wait for NACK
    if (WaitForPacket(MSG_ID_NACK, 10, "Test3") == -1) {
        return;
    }

    // Valid size, invalid address
    msg_id = MSG_ID_ACK;
    ToFlashPacket(0x8003000, 0x800, data);
    ComTransmitPacket(MSG_ID_MEM_WRITE, data, 6);
    // Wait for NACK
    qint64 elapsed = WaitForPacket(MSG_ID_NACK, 10, "Test3");
    if (elapsed == -1) {
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
    if (WaitForPacket(MSG_ID_ACK, 100, "Test4") == -1) {
        return;
    }

    // Attempt to write data
    // Send mem write request
    ToFlashPacket(0x8005000, 0x8, data);
    ComTransmitPacket(MSG_ID_MEM_WRITE, data, 6);
    // Wait for ACK
    if (WaitForPacket(MSG_ID_ACK, 20, "Test4") == -1) {
        return;
    }
    // Send write data
    uint8_t write_data[8] = {0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B};
    ComTransmit(write_data, 8, 100);
    // Send checksum
    uint32_t checksum = crc32(write_data, 8, INITIAL_CRC);
    ComTransmit((uint8_t*)&checksum, 4, 100);
    // Wait for ACK
    qint64 elapsed = WaitForPacket(MSG_ID_ACK, 200, "Test4");
    if (elapsed == -1) {
        return;
    }

    // Send read request to read back written data
    ToFlashPacket(0x8005000, 0x8, data);
    ComTransmitPacket(MSG_ID_MEM_READ, data, 6);
    // Wait for ACK
    if (WaitForPacket(MSG_ID_ACK, 100, "Test4") == -1) {
        return;
    }
    // Read back written data
    if (ComReceive(data, 0x8, 1000) != BOOT_OK) {
        qDebug() << "Test4 Error: Readback failed";
        return;
    }
    if (memcmp(data, write_data, 8) != 0) {
        qDebug() << "Test4 Error: Write failed";
        return;
    }
    // Wait for ACK
    if (WaitForPacket(MSG_ID_ACK, 10, "Test4") == -1) {
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

    Test1();
    Test2();
    Test3();
    Test4();
    //Test5();

    return a.exec();
}
