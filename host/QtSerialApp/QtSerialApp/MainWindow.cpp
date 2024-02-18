#include "MainWindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _serial.Connect("COM3", "9600", "8", "E", "1", "None");
}

MainWindow::~MainWindow()
{
    delete ui;
}
