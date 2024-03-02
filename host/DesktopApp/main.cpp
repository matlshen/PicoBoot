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

    QObject::connect(&worker, &IoThread::SendLog, &w, &MainWindow::UpdateLog);

    // Start thread
    io_thread.start();

    return a.exec();
}
