#include "mainwindow.h"

#include <QApplication>
#include "IoThread.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    // Spawn worker in thread
    QThread io_thread;
    IoThread worker;
    worker.moveToThread(&io_thread);

    // Connect signals and slots
    QObject::connect(&io_thread, &QThread::started, &worker, &IoThread::work);

    QObject::connect(&w, &MainWindow::ConnectSignal, &worker, &IoThread::ConnectSlot);
    QObject::connect(&w , &MainWindow::GetConfigSignal, &worker, &IoThread::GetConfigSlot);
    QObject::connect(&w, &MainWindow::EraseSignal, &worker, &IoThread::EraseSlot);
    QObject::connect(&w, &MainWindow::ReadSignal, &worker, &IoThread::ReadSlot);
    QObject::connect(&w, &MainWindow::ReadFileSignal, &worker, &IoThread::GetFileDataSlot);
    QObject::connect(&w, &MainWindow::DownloadSignal, &worker, &IoThread::DownloadSlot);
    QObject::connect(&w, &MainWindow::VerifySignal, &worker, &IoThread::VerifySlot);
    QObject::connect(&w, &MainWindow::GoSignal, &worker, &IoThread::GoSlot);
    QObject::connect(&w, &MainWindow::ResetSignal, &worker, &IoThread::ResetSlot);

    QObject::connect(&worker, &IoThread::SendLog, &w, &MainWindow::UpdateLog);
    QObject::connect(&worker, &IoThread::UpdateProgress, &w, &MainWindow::UpdateProgress);

    // Start thread
    io_thread.start();


    // // Open port
    // _serial.setPortName("COM15");
    // _serial.setBaudRate(QSerialPort::Baud115200);
    // _serial.setDataBits(QSerialPort::Data8);
    // _serial.setParity(QSerialPort::NoParity);
    // _serial.setStopBits(QSerialPort::OneStop);
    // _serial.setFlowControl(QSerialPort::NoFlowControl);
    // if (_serial.open(QIODevice::ReadWrite)) {
    //     qDebug("Opened port COM13");
    // } else {
    //     qDebug("Failed to open port COM13");
    // }

    // uint8_t rx_data[100];
    // while (1) {
    //     if (UARTReceive(rx_data, 5, 500) == BOOT_OK)
    //         UARTTransmit(rx_data, 5, 1000);

    //     QThread::msleep(1000);
    // }

    return a.exec();
}
