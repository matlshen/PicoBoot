#ifndef IOTHREAD_H
#define IOTHREAD_H

#include <QObject>
#include <QThread>
#include <QSerialPort>
#include <QBrush>

extern "C" void UART_RxCallback(uint8_t *buf, uint32_t len);

class IoThread : public QObject
{
    Q_OBJECT
public:
    explicit IoThread(QObject *parent = nullptr);
public slots:
    void work();
    void ConnectSlot(QString portName, int nodeId = -1);
    void GetConfigSlot();
    void EraseSlot(uint32_t address, uint16_t size);
    void ReadSlot(uint32_t address, uint16_t size);
    void SwapSlot(uint8_t src_slot, uint8_t dst_slot);
    void GetFileDataSlot(QString fielname);
    void DownloadSlot(uint8_t slot);
    void VerifySlot(uint8_t slot);
    void GoSlot();
    void ResetSlot();
signals:
    void SendLog(QString msg, const QBrush& color = Qt::black);
    void UpdateProgress(int progress);

public:
    QByteArray _data;
    int _data_size;
    QByteArray _data_hash;
};

#endif // IOTHREAD_H
