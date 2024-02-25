#include <QCoreApplication>
#include <QElapsedTimer>
#include <QDebug>
#include <QtAssert>
#include "uart.h"

/* Receive 2 6 byte sequences over UART. Confirm that both sequences contain
 the message "error" and that they are received about 50ms apart */
void Test1() {
    QElapsedTimer timer;

    uint8_t data[6] = {0};

    UARTReceive(data, 6, 1000);
    timer.start();
    if (strcmp((char*)data, "error") != 0) {
        qDebug() << "Test1: Error message 1 not received";
        return;
    }

    UARTReceive(data, 6, 1000);
    qint64 elapsed_time = timer.elapsed();

    if (strcmp((char*)data, "error") != 0) {
        qDebug() << "Test1: Error message 2 not received";
        return;
    }

    if (elapsed_time < 50 * 0.9) {
        qDebug() << "Test1: Timeout on target expired too quickly (" << elapsed_time << "ms)";
        return;
    }

    if (elapsed_time > 50 * 1.1) {
        qDebug() << "Test1: Timeout on target expired too slowly (" << elapsed_time << "ms)";
        return;
    }

    qDebug() << "Test1: Passed";
}

/* Echo message on UART */
void Test2() {
    QElapsedTimer timer;
    uint8_t data[6] = {0};

    UARTTransmit((uint8_t*)"tests", 6);
    timer.start();

    UARTReceive(data, 6, 1000);
    qint64 elapsed_time = timer.elapsed();

    if (strcmp((char*)data, "tests") != 0) {
        qDebug() << "Test2: Received message does not match transmitted message" << data;
        return;
    }

    qDebug() << "Test2: Passed" << ", UART Latency:" << elapsed_time << "ms";
}

/* Send wrong number of bytes */
void Test3() {
    QElapsedTimer timer;
    uint8_t data[6] = {0};

    UARTTransmit((uint8_t*)"one", 4);
    timer.start();

    UARTReceive(data, 6, 1000);
    qint64 elapsed_time = timer.elapsed();

    if (strcmp((char*)data, "error") != 0) {
        qDebug() << "Test3: Error message not received" << data;
        return;
    }

    if (elapsed_time < UART_BYTE_TIMEOUT_MS * 0.9) {
        qDebug() << "Test3: Timeout on target expired too quickly (" << elapsed_time << "ms)";
        return;
    }

    if (elapsed_time < UART_BYTE_TIMEOUT_MS * 0.9) {
        qDebug() << "Test3: Timeout on target expired too quickly (" << elapsed_time << "ms)";
        return;
    }

    qDebug() << "Test3: Passed";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    UARTInit();

    Test1();
    Test2();
    Test3();

    return a.exec();
}
