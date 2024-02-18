#include "uart.h"
#include <QSerialPort>
#include <QDebug>

QSerialPort _serial;

Boot_StatusTypeDef UARTInit(void) {
    _serial.setPortName("COM1");    // TODO: Make this configurable
    _serial.setBaudRate(QSerialPort::Baud115200);
    _serial.setDataBits(QSerialPort::Data8);
    _serial.setParity(QSerialPort::EvenParity);
    _serial.setStopBits(QSerialPort::OneStop);
    _serial.setFlowControl(QSerialPort::NoFlowControl);

    // Set the read buffer size to 1 so that we can read one byte at a time
    _serial.setReadBufferSize(1);

    if (_serial.open(QIODevice::ReadWrite)) {
        return BOOT_OK;
    } else {
        return BOOT_ERROR;
    }
}

Boot_StatusTypeDef UARTDeInit(void) {
    _serial.close();
    return BOOT_OK;
}

Boot_StatusTypeDef UARTTransmit(uint8_t *data, uint8_t length, uint32_t timeout_ms) {
    for (int i = 0; i < length; i++) {
        // Timeout ignored for now
        if (_serial.write((const char *)data, 1) == 1) {
            // Wait for the data to be written to the serial port
            if (!_serial.waitForBytesWritten(timeout_ms)) {
                qDebug() << "UART Tx Timeout";
                return BOOT_TIMEOUT;
            }

            qDebug() << "UART Byte Sent";

        // If the write fails, return an error
        } else {
            qDebug() << "UART Tx Error";
            return BOOT_ERROR;
        }
    }

    return BOOT_OK;
}

Boot_StatusTypeDef UARTReceive(uint8_t *data, uint8_t length, uint32_t timeout_ms) {
    int received = 0;
    for (int i = 0; i < length; i++) {

        // For the first byte, wait for the specified timeout period
        // For subsequent bytes, wait for the byte timeout period
        if (_serial.waitForReadyRead(received == 0 ? timeout_ms : UART_BYTE_TIMEOUT_MS)) {
            received += _serial.read((char *)data + i, 1);
        } else {
            qDebug() << "UART Rx Timeout";
            return BOOT_TIMEOUT;
        }

        qDebug() << "UART Byte Received";
    }

    return BOOT_OK;
}
