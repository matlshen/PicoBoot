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
    void EraseSlot(uint32_t address, uint16_t size);
    void ReadSlot(uint32_t address, uint16_t size);
signals:
    void SendLog(QString msg, const QBrush& color = Qt::black);

public:
    static QSerialPort* _serial;
};

#endif // IOTHREAD_H
