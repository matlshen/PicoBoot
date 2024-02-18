#ifndef BOOTTARGET_H
#define BOOTTARGET_H

#include <QObject>
#include "SerialPort.h"

class BootTarget : public QObject
{
    Q_OBJECT
public:
    explicit BootTarget(QObject *parent = nullptr);
    bool Connect();

signals:
private:
    SerialPort _serial;
};

#endif // BOOTTARGET_H
