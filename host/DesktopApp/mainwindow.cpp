#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include "host.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_browseBtn_clicked()
{
    QString filter = "Binary files (*.bin);;Hex files (*.hex)";
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Open File",
                                                    QDir::homePath(),
                                                    filter);
    if (!filename.isEmpty()) {
        ui->fileLineEdit->setText(filename);
    }
}

