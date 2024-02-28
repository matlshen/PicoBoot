#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>
#include <QDebug>
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
    uint8_t write_data[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78};
    qint64 elapsed = -1;

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
    if (ComTransmit(write_data, 8, 100) != BOOT_OK) {
        qDebug() << "Test4 Error: Failed to send write data";
        return;
    }
    // Send checksum
    uint32_t checksum = crc32(write_data, 8, INITIAL_CRC);
    if (ComTransmit((uint8_t*)&checksum, 4, 100) != BOOT_OK) {
        qDebug() << "Test4 Error: Failed to send checksum";
        return;
    }
    // Wait for ACK
    elapsed = WaitForPacket(MSG_ID_ACK, 200, "Test4");
    if (elapsed == -1) {
        return;
    }

    // Send read request to read back written data
    ToFlashPacket(0x8005000, 0x8, data);
    ComTransmitPacket(MSG_ID_MEM_READ, data, 6);
    // Wait for ACK
    if (WaitForPacket(MSG_ID_ACK, 5000, "Test4") == -1) {
        return;
    }
    // Read back written data
    if (ComReceive(data, 0x8, 5000) != BOOT_OK) {
        qDebug() << "Test4 Error: Readback failed";
        return;
    }
    if (memcmp(data, write_data, 8) != 0) {
        qDebug() << "Test4 Error: Write failed";
        return;
    }
    // Wait for ACK
    if (WaitForPacket(MSG_ID_ACK, 1000, "Test4") == -1) {
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

void Test6() {
    uint8_t read_data[8];
    if (ReadTargetMemory(0x8005000, 0x8, read_data) != BOOT_OK) {
        qDebug() << "Test6 Error: Read failed";
        return;
    }

    // Print read data
    for (int i = 0; i < 8; i++) {
        qDebug() << Qt::hex << read_data[i];
    }
}

void Test7() {
    uint8_t write_data[8] = {0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};
    if (WriteTargetMemory(0x8005018, 0x8, write_data) != BOOT_OK) {
        qDebug() << "Test7 Error: Write failed";
        return;
    }

    uint8_t read_data[8];
    if (ReadTargetMemory(0x8005018, 0x8, read_data) != BOOT_OK) {
        qDebug() << "Test7 Error: Read failed";
        return;
    }

    // Check if read data matches written data
    if (memcmp(write_data, read_data, 8) != 0) {
        qDebug() << "Test7 Error: Read data does not match written data";
        return;
    }

    qDebug() << "Test7: Passed";
}

void Test8() {
    uint8_t write_data[0x130];
    for (int i = 0; i < 0x130; i++) {
        write_data[i] = i % 256;
    }

    // Erase segment
    if (EraseTargetMemory(0x8005000, 0x800) != BOOT_OK) {
        qDebug() << "Erase failed";
        return;
    }

    // Write to memory
    QElapsedTimer timer;
    timer.start();
    if (WriteTargetMemory(0x8005000, 0x130, write_data) != BOOT_OK) {
        qDebug() << "Test8 Error: Write failed";
        return;
    }
    qint64 elapsed = timer.elapsed();

    // Read back write data
    uint8_t read_data[0x130];
    if (ReadTargetMemory(0x8005000, 0x130, read_data) != BOOT_OK) {
        qDebug() << "Test8 Error: Read failed";
        return;
    }

    // Check if read data matches written data
    if (memcmp(write_data, read_data, 0x130) != 0) {
        qDebug() << "Test8 Error: Read data does not match written data";
        return;
    }

    qDebug() << "Test8: Passed" << ", write latency:" << elapsed << "ms";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ComInit();

    // Flash tests
    //Test1();
    //Test2();
    //Test3();
    //Test4();
    //Test5();

    // host.c tests
    Test6();
    Test7();
    Test8();

    return a.exec();
}
