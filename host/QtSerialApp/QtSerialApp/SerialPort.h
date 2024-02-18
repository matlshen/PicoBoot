#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QSerialPort>
#include <QMessageBox>

class SerialPort : public QObject
{
    Q_OBJECT
public:
    explicit SerialPort(QObject *parent = nullptr);
    ~SerialPort();
    bool Connect(QString port_name,
                 QString baudrate,
                 QString data_bits,
                 QString parity,
                 QString stop_bits,
                 QString flow_control);
    bool Write(QByteArray data);

signals:
    void DataReceived(QByteArray bytearray);

private slots:
    void ReadyReadHandler();

private:
    QSerialPort *m_serial;


};

#endif // SERIALPORT_H
