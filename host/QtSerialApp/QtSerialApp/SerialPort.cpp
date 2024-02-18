#include "SerialPort.h"

SerialPort::SerialPort(QObject *parent)
    : QObject{parent}
{
    m_serial = nullptr;
}

SerialPort::~SerialPort() {
    if (m_serial != nullptr) {
        m_serial->close();
        delete m_serial;
    }
}

bool SerialPort::Connect(QString port_name,
                         QString baudrate,
                         QString data_bits,
                         QString parity,
                         QString stop_bits,
                         QString flow_control) {
    if (m_serial != nullptr) {
        m_serial->close();
        delete m_serial;
    }

    QSerialPort::DataBits q_data_bits;
    switch(data_bits.toInt()) {
    case 5:
        q_data_bits = QSerialPort::Data5;
        break;
    case 6:
        q_data_bits = QSerialPort::Data6;
        break;
    case 7:
        q_data_bits = QSerialPort::Data7;
        break;
    case 8:
        q_data_bits = QSerialPort::Data8;
        break;
    default:
        return false;
    }

    QSerialPort::Parity q_parity;
    switch(parity[0].toLatin1()) {
    case 'N':
        q_parity = QSerialPort::NoParity;
        break;
    case 'E':
        q_parity = QSerialPort::EvenParity;
        break;
    case 'O':
        q_parity = QSerialPort::OddParity;
        break;
    default:
        return false;
    }

    QSerialPort::StopBits q_stop_bits;
    switch(stop_bits.toInt()) {
    case 1:
        q_stop_bits = QSerialPort::StopBits::OneStop;
        break;
    case 2:
        q_stop_bits = QSerialPort::StopBits::TwoStop;
        break;
    default:
        return false;
    }

    QSerialPort::FlowControl q_flow_control;
    switch(flow_control[0].toLatin1()) {
    case 'N':
        q_flow_control = QSerialPort::FlowControl::NoFlowControl;
        break;
    case 'R':
        q_flow_control = QSerialPort::FlowControl::HardwareControl;
        break;
    case 'X':
        q_flow_control = QSerialPort::FlowControl::SoftwareControl;
        break;
    default:
        return false;
    }


    m_serial = new QSerialPort(this);
    m_serial->setPortName(port_name);
    m_serial->setBaudRate(baudrate.toInt());
    m_serial->setDataBits(q_data_bits);
    m_serial->setParity(q_parity);
    m_serial->setStopBits(q_stop_bits);
    m_serial->setFlowControl(q_flow_control);

    if (m_serial->open(QIODevice::ReadWrite)) {
        QObject::connect(m_serial, &QSerialPort::readyRead,
                         this, &SerialPort::ReadyReadHandler);
    }

    return m_serial->isOpen();
}

bool SerialPort::Write(QByteArray data) {
    if (m_serial == nullptr || !m_serial->isOpen()) {
        return false;
    }

    return m_serial->write(data) == data.size();
}

void SerialPort::ReadyReadHandler() {
    if (!m_serial->isOpen() || !m_serial->isReadable()) {
        return;
    }
    emit DataReceived(m_serial->readAll());
}
