#include "uart.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QByteArray>
#include <QThread>
#include <QDebug>

// Assuming there's a global QSerialPort object for simplicity
QSerialPort* _serial;
QMutex serialMutex;

// Circular buffer for overflow data
class CircularBuffer {
public:
    static const int BufferSize = 1024; // Adjust size as needed
    uint8_t buffer[BufferSize];
    int head = 0;
    int tail = 0;
    int size = 0;

    void put(const uint8_t *data, int len) {
        QMutexLocker locker(&serialMutex);
        for (int i = 0; i < len; ++i) {
            buffer[head] = data[i];
            head = (head + 1) % BufferSize;
            if (size < BufferSize) {
                size++;
            } else {
                tail = (tail + 1) % BufferSize; // Overwrite if buffer is full
            }
        }
    }

    int get(uint8_t *data, int len) {
        QMutexLocker locker(&serialMutex);
        int count = 0;
        while (size > 0 && count < len) {
            data[count++] = buffer[tail];
            tail = (tail + 1) % BufferSize;
            size--;
        }
        return count; // Number of bytes read
    }

    void clear() {
        size = 0;
        head = 0;
        tail = 0;
    }

    bool isEmpty() const {
        return size == 0;
    }
};

CircularBuffer overflowBuffer;

// Leave initialization to application
Boot_StatusTypeDef UARTInit(void) {
    return BOOT_OK;
}

// Deinitialize serial port
Boot_StatusTypeDef UARTDeInit(void) {
    _serial->close();
    return BOOT_OK;
}

// Transmit data
Boot_StatusTypeDef UARTTransmit(const uint8_t *data, uint32_t length, uint32_t timeout_ms) {
    QMutexLocker locker(&serialMutex);

    if (length == 0) {
        return BOOT_OK;
    }

    qint64 bytesWritten = _serial->write(reinterpret_cast<const char*>(data), length);
    if (!_serial->waitForBytesWritten(timeout_ms) || bytesWritten != length) {
        qDebug() << "Transmission timeout or error";
        qDebug() << "Requested" << length << "bytes but " << bytesWritten;
        return BOOT_TIMEOUT;
    }

    // Clear overflow buffer
    overflowBuffer.clear();
    _serial->clear();

    return BOOT_OK;
}

// Receive data
Boot_StatusTypeDef UARTReceive(uint8_t *data, uint32_t length, uint32_t timeout_ms) {
    QElapsedTimer timer;
    int bytesRead = 0;
    qDebug() << "Call with parameter: " << length << "timeout_ms: " << timeout_ms;

    // First, try to fulfill the request from the overflow buffer
    bytesRead += overflowBuffer.get(data, length);
    if (bytesRead == length) {
        qDebug() << "Overflow Received: " << bytesRead << "bytes " << *data;
        return BOOT_OK; // Request fulfilled from overflow buffer
    }

    // If the overflow buffer was not enough, read the rest from the serial port
    timer.start();
    while (bytesRead < length && timer.elapsed() < timeout_ms) {
        if (_serial->waitForReadyRead(timeout_ms - timer.elapsed())) {
            QByteArray readData = _serial->read(_serial->bytesAvailable());
            int readNow = readData.size();
            qDebug() << "Read size: " << readNow << "bytes";
            if (readNow > 0) {
                // If received more than needed, put the excess in the overflow buffer
                if (bytesRead + readNow > length) {
                    int excess = bytesRead + readNow - length;
                    overflowBuffer.put(reinterpret_cast<const uint8_t*>(readData.constData()) + (readNow - excess), excess);
                    readNow -= excess;
                }
                memcpy(data + bytesRead, readData.constData(), readNow);
                bytesRead += readNow;
            }
        } else {
            qDebug() << "UARTReceive timeout";
            return BOOT_TIMEOUT;
        }
    }

    if (bytesRead < length) {
        qDebug() << "UARTReceive timeout";
        // Clear overflow buffer if request was not fulfilled
        overflowBuffer.clear();
        return BOOT_TIMEOUT;
    }

    qDebug() << "Received: " << bytesRead << "bytes " << *data;
    return bytesRead == length ? BOOT_OK : BOOT_TIMEOUT;
}
