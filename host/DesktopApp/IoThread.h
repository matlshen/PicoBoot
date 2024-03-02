#ifndef IOTHREAD_H
#define IOTHREAD_H

#include <QObject>
#include <QThread>
#include <QSerialPort>
#include <QBrush>

class IoThread : public QObject
{
    Q_OBJECT
public:
    explicit IoThread(QObject *parent = nullptr);
public slots:
    void work();
    void ConnectSlot(QString portName);
    void GetConfigSlot();
    void EraseSlot(uint32_t address, uint16_t size);
    void ReadSlot(uint32_t address, uint16_t size);
    void GetFileDataSlot(QString fielname);
    void DownloadSlot();
    void VerifySlot(uint8_t slot);
    void GoSlot();
    void ResetSlot();
signals:
    void SendLog(QString msg, const QBrush& color = Qt::black);
    void UpdateProgress(int progress);

public:
    static QSerialPort* _serial;
    QByteArray _data;
    int _data_size;
    QByteArray _data_hash;
};

#endif // IOTHREAD_H
