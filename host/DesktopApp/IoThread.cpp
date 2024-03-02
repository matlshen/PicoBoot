#include "IoThread.h"
#include "host.h"

QSerialPort* IoThread::_serial = nullptr;

IoThread::IoThread(QObject *parent) : QObject(parent) {
    if (_serial == nullptr) _serial = new QSerialPort(this);
}

void IoThread::work() {
    // while (1) {
    //     qDebug("Hello from thread");
    //     QThread::msleep(1000);
    // }
}

void IoThread::ConnectSlot(QString portName) {
    qDebug("Opening port %s", portName.toStdString().c_str());

    // Close serial port if it is already open
    if (_serial->isOpen()) {
        _serial->close();
    }

    // Open port (instead of calling ComInit())
    _serial->setPortName(portName);
    _serial->setBaudRate(QSerialPort::Baud115200);
    _serial->setDataBits(QSerialPort::Data8);
    _serial->setParity(QSerialPort::OddParity);
    _serial->setStopBits(QSerialPort::OneStop);
    _serial->setFlowControl(QSerialPort::NoFlowControl);

    // Set minimum read buffer size
    IoThread::_serial->setReadBufferSize(1);

    // Open serial port
    if (_serial->open(QIODevice::ReadWrite)) {
        emit SendLog("Successfully opened port " + portName);
    } else {
        emit SendLog("Failed to open port " + portName);
    }


    // Connect to target
    Boot_StatusTypeDef status = ConnectToTarget();
    if (status == BOOT_OK)
        emit SendLog("Connected to target");
    else if (status == BOOT_TIMEOUT)
        emit SendLog("Target timed out when attempting to connect", Qt::red);
    else
        emit SendLog("Error connecting to target", Qt::red);

    // if (status == BOOT_OK)
    //     qDebug("Connected to target");
    // else if (status == BOOT_TIMEOUT)
    //     qDebug("ConnectToTarget() timeout");
    // else if (status == BOOT_ERROR)
    //     qDebug("ConnectToTarget() error");
}

void IoThread::GetConfigSlot() {
    Boot_ConfigTypeDef config;
    Boot_StatusTypeDef status = GetTargetConfig(&config);

    if (status == BOOT_OK)
        emit SendLog("Successfully retrieved target config");
    else if (status == BOOT_TIMEOUT)
        emit SendLog("GetConfig operation timed out", Qt::red);
    else
        emit SendLog("Error getting target config", Qt::red);
}

void IoThread::EraseSlot(uint32_t address, uint16_t size) {
    Boot_StatusTypeDef status = EraseTargetMemory(address, size);

    if (status == BOOT_OK)
        emit SendLog("Erase operation successful");
    else if (status == BOOT_TIMEOUT)
        emit SendLog("Erase operation timed out", Qt::red);
    else
        emit SendLog("Error erasing memory", Qt::red);
}

void IoThread::ReadSlot(uint32_t address, uint16_t size) {
    uint8_t* data = new uint8_t[size];
    Boot_StatusTypeDef status = ReadTargetMemory(address, size, data);

    if (status == BOOT_OK) {
        emit SendLog("Read operation successful");
        // Convert bytes to hex string
        QString hexString;
        for (int i = 0; i < size; i++) {
            hexString.append(QString("%1").arg(data[i], 2, 16, QChar('0')).toUpper());
            if ((i+9) % 8 != 0)
                hexString.append(" ");
            else if (i != size-1)
                hexString.append("\n");
        }
        emit SendLog(hexString);
    } else if (status == BOOT_TIMEOUT)
        emit SendLog("Read operation timed out", Qt::red);
    else
        emit SendLog("Error reading memory", Qt::red);

    delete[] data;
}
