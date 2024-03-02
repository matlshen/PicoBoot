#include "uart.h"
#include <QSerialPort>
#include <QDebug>
#include "IoThread.h"

// This function is only used for testing
// In host app, the desktop app will handle opening the port
Boot_StatusTypeDef UARTInit(void) {
    IoThread::_serial->setPortName("COM13");
    IoThread::_serial->setBaudRate(QSerialPort::Baud115200);
    IoThread::_serial->setDataBits(QSerialPort::Data8);
    IoThread::_serial->setParity(QSerialPort::NoParity);
    IoThread::_serial->setStopBits(QSerialPort::OneStop);
    IoThread::_serial->setFlowControl(QSerialPort::NoFlowControl);

    // Set the read buffer size to 1 so that we can read one byte at a time
    IoThread::_serial->setReadBufferSize(1);

    if (IoThread::_serial->open(QIODevice::ReadWrite)) {
        return BOOT_OK;
    } else {
        return BOOT_ERROR;
    }
}

Boot_StatusTypeDef UARTDeInit(void) {
    IoThread::_serial->close();
    return BOOT_OK;
}

Boot_StatusTypeDef UARTTransmit(const uint8_t *data, uint32_t length, uint32_t timeout_ms) {
    for (int i = 0; i < length; i++) {

        // Attempt to write the byte to the serial port
        if (IoThread::_serial->write((const char *)data+i, 1) == 1) {
            // Wait for the data to be written to the serial port
            if (!IoThread::_serial->waitForBytesWritten(timeout_ms)) {
                // qDebug() << "UART Tx Timeout";
                return BOOT_TIMEOUT;
            }

            // qDebug() << "UART Byte Sent";

        // If the write fails, return an error
        } else {
            // qDebug() << "UART Tx Error";
            return BOOT_ERROR;
        }
    }

    IoThread::_serial->clear();

    return BOOT_OK;
}

Boot_StatusTypeDef UARTReceive(uint8_t *data, uint32_t length, uint32_t timeout_ms) {
    int received = 0;
    for (int i = 0; i < length; i++) {

        // For the first byte, wait for the specified timeout period
        // For subsequent bytes, wait for the byte timeout period
        if (IoThread::_serial->waitForReadyRead(received == 0 ? timeout_ms : UART_BYTE_TIMEOUT_MS)) {
            received += IoThread::_serial->read((char *)data + i, 1);
        } else {
            // qDebug() << "UART Rx Timeout";
            return BOOT_TIMEOUT;
        }

        // qDebug() << "UART Byte Received" << Qt::hex << data[i];
    }

    return BOOT_OK;
}
