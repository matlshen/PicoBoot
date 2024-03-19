#include "IoThread.h"
#include "host.h"
#include <QFile>
#include <QCryptographicHash>

extern QSerialPort* _serial;

IoThread::IoThread(QObject *parent) : QObject(parent) {

}

void IoThread::work() {
    // while (1) {
    //     qDebug("Hello from thread");
    //     QThread::msleep(1000);
    // }
}

void IoThread::ConnectSlot(QString portName, int nodeId) {
    qDebug("Opening port %s", portName.toStdString().c_str());

    if (_serial == nullptr)
        _serial = new QSerialPort();

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

    // Open serial port
    if (_serial->open(QIODevice::ReadWrite)) {
        emit SendLog("Opened port " + portName, Qt::blue);
    } else {
        emit SendLog("Failed to open port " + portName, Qt::red);
    }

    // Attempt to connect for 5 seconds
    Boot_StatusTypeDef status;
    int num_attempts = 5000 / BL_TIMEOUT_MS;
    for (int i = 0; i < num_attempts; i++) {
        emit SendLog("Attempting to connect to target", Qt::black);
        status = ConnectToTarget(nodeId == -1 ? 0xFFFF : nodeId);
        if (status == BOOT_OK)
            break;
    }

    // Check connection status
    if (status == BOOT_OK) {
        emit SendLog("Connected to target", Qt::blue);
        // Get target configuration
        GetConfigSlot();
    }
    else if (status == BOOT_TIMEOUT)
        emit SendLog("Target timed out when attempting to connect", Qt::red);
    else
        emit SendLog("Error connecting to target", Qt::red);
}

void IoThread::GetConfigSlot() {
    Boot_StatusTypeDef status = GetTargetConfig();

    if (status == BOOT_OK) {
        emit SendLog("Retrieved target configuration", Qt::blue);
        emit SendLog("bootloader version: " + QString::number(target_config.version.major) + "." + QString::number(target_config.version.minor));
        for (int i = 0; i < BL_NUM_SLOTS; i++) {
            emit SendLog("slot " + QString::number(i) + " address: 0x" + QString::number(target_config.slot_list[i].load_address, 16).toUpper());
            emit SendLog("slot " + QString::number(i) + " size: 0x" + QString::number(target_config.slot_list[i].slot_size, 16).toUpper());
        }
    }
    else if (status == BOOT_TIMEOUT)
        emit SendLog("GetConfig operation timed out", Qt::red);
    else
        emit SendLog("Error getting target config", Qt::red);
}

void IoThread::EraseSlot(uint32_t address, uint16_t size) {
    Boot_StatusTypeDef status = EraseTargetMemory(address, size);

    if (status == BOOT_OK)
        emit SendLog("Erase operation successful", Qt::blue);
    else if (status == BOOT_TIMEOUT)
        emit SendLog("Erase operation timed out", Qt::red);
    else
        emit SendLog("Error erasing memory", Qt::red);
}

void IoThread::ReadSlot(uint32_t address, uint16_t size) {
    uint8_t* data = new uint8_t[size];
    Boot_StatusTypeDef status = ReadTargetMemory(address, size, data);

    if (status == BOOT_OK) {
        emit SendLog("Target read data:", Qt::blue);
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

void IoThread::SwapSlot(uint8_t src_slot, uint8_t dst_slot) {
    Boot_StatusTypeDef status = SwapTarget(src_slot, dst_slot);

    if (status == BOOT_OK)
        emit SendLog("Swap operation successful", Qt::blue);
    else if (status == BOOT_TIMEOUT)
        emit SendLog("Swap operation timed out", Qt::red);
    else
        emit SendLog("Error swapping slots", Qt::red);
}

void IoThread::GetFileDataSlot(QString filename) {
    if (filename.isEmpty())
        return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        emit SendLog("Error opening file " + filename, Qt::red);
        return;
    }

    // Read binary data from file
    _data = file.readAll();
    _data_size = _data.size();
    _data_hash = QCryptographicHash::hash(_data, QCryptographicHash::Sha256);
    file.close();

    emit SendLog("Binary size: " + QString::number(_data_size));
    emit SendLog("Binary hash: " + _data_hash.toHex());
}

void IoThread::DownloadSlot(uint8_t slot) {
    if (_data.isEmpty()) {
        emit SendLog("No data to download", Qt::red);
        return;
    }

    // Erase section of memory where application will be written
    Boot_StatusTypeDef status = EraseTargetMemory(target_config.slot_list[slot].load_address, FlashUtil_RoundUpToPage(_data_size));
    if (status != BOOT_OK) {
        emit SendLog("Error erasing memory", Qt::red);
        return;
    }

    // Write binary data to target in 256 byte chunks
    int num_chunks = _data_size / 256;
    for (int i = 0; i < num_chunks; i++) {
        status = WriteTargetMemory(target_config.slot_list[slot].load_address + i*256, 256, (uint8_t*)_data.data() + i*256);
        if (status == BOOT_TIMEOUT) {
            emit SendLog("Download operation timed out", Qt::red);
            return;
        }
        else if (status != BOOT_OK) {
            emit SendLog("Error writing memory", Qt::red);
            return;
        }

        // Update progress bar
        emit UpdateProgress((i+1) * 100 / num_chunks);
    }
    // Write remaining data
    status = WriteTargetMemory(target_config.slot_list[slot].load_address + num_chunks*256, _data_size - num_chunks*256, (uint8_t*)_data.data() + num_chunks*256);
    if (status == BOOT_OK)
        emit SendLog("Download operation successful", Qt::blue);
    else if (status == BOOT_TIMEOUT)
        emit SendLog("Download operation timed out", Qt::red);
    else
        emit SendLog("Error writing memory", Qt::red);

    emit UpdateProgress(100);



    // Write new configuration to target
    // Image size
    target_config.slot_list[slot].image_size = _data_size;

    // MSP and reset vector
    target_config.slot_list[slot].msp = ((uint32_t*)_data.data())[0];
    target_config.slot_list[slot].reset_vector = ((uint32_t*)_data.data())[1];

    // Write hash to target config
    memcpy(target_config.slot_list[slot].hash, _data_hash, 32);

    // Compute CRC32 of new configuration
    target_config.crc32 = crc32((uint8_t*)&target_config+4, sizeof(target_config)-4, INITIAL_CRC);

    // Write new configuration to target
    status = SetTargetConfig();
    if (status == BOOT_OK)
        emit SendLog("Wrote new configuration to target", Qt::blue);
    else if (status == BOOT_TIMEOUT)
        emit SendLog("SetConfig operation timed out", Qt::red);
    else
        emit SendLog("Error writing new configuration to target", Qt::red);
}

void IoThread::VerifySlot(uint8_t slot) {
    Boot_StatusTypeDef status = VerifyTarget(slot);
    if (status == BOOT_OK)
        emit SendLog("Verification successful", Qt::blue);
    else if (status == BOOT_TIMEOUT)
        emit SendLog("Verification operation timed out", Qt::red);
    else
        emit SendLog("Verification failed", Qt::red);
}

void IoThread::GoSlot() {
    Boot_StatusTypeDef status = GoTarget();
    if (status == BOOT_OK)
        emit SendLog("Application started", Qt::blue);
    else if (status == BOOT_TIMEOUT)
        emit SendLog("Go operation timed out", Qt::red);
    else
        emit SendLog("Error starting application", Qt::red);
}

void IoThread::ResetSlot() {
    Boot_StatusTypeDef status = ResetTarget();
    if (status == BOOT_OK)
        emit SendLog("Target reset", Qt::blue);
    else if (status == BOOT_TIMEOUT)
        emit SendLog("Reset operation timed out", Qt::red);
    else
        emit SendLog("Error resetting target", Qt::red);
}
