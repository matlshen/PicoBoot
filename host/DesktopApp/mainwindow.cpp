#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    LoadPorts();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::UpdateLog(QString msg, const QBrush& color) {
    QTextCursor cursor = ui->logTextEdit->textCursor();

    // Set text color
    QTextCharFormat format;
    format.setForeground(color);
    cursor.mergeCharFormat(format);

    cursor.insertText(msg);
    cursor.insertText("\n");

    // Scroll to bottom
    ui->logTextEdit->ensureCursorVisible();
}

void MainWindow::UpdateProgress(int progress) {
    ui->progressBar->setValue(progress);
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
    emit ReadFileSignal(filename);
}

void MainWindow::LoadPorts() {
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (auto& port : ports) {
        ui->portComboBox->addItem(port.portName());
    }
}

void MainWindow::on_connectBtn_clicked()
{
    QString portName = ui->portComboBox->currentText();
    emit ConnectSignal(portName);
}


void MainWindow::on_lineEdit_returnPressed()
{
    // Retrive the command line text
    QString command = ui->lineEdit->text();

    // Clear the command line
    ui->lineEdit->clear();

    // Tokenize the command
    QStringList tokens = command.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

    // If the command is empty, return
    if (tokens.isEmpty()) return;

    // If the command is "connect", call the connect button slot
    if (tokens.at(0) == "connect") {
        on_connectBtn_clicked();
    }
    else if (tokens.at(0) == "getconfig") {
        emit GetConfigSignal();
    }
    else if (tokens.at(0) == "erase") {
        if (tokens.size() != 3) {
            UpdateLog("Usage: erase <address> <size>");
            return;
        }

        // Remove "0x" from the address
        if (tokens.at(1).startsWith("0x")) {
            tokens[1] = tokens[1].mid(2);
        }
        if (tokens.at(2).startsWith("0x")) {
            tokens[2] = tokens[2].mid(2);
        }

        // Convert the address and size to integers
        uint32_t address = tokens.at(1).toUInt(nullptr, 16);
        uint16_t size = tokens.at(2).toUInt(nullptr, 16);

        // Emit the erase signal
        emit EraseSignal(address, size);
    }
    else if (tokens.at(0) == "read") {
        // Read 8 bytes if size not specified
        if (tokens.size() < 2) {
            UpdateLog("Usage: read <address> [size]");
        }

        // Remove "0x" from the address
        if (tokens.at(1).startsWith("0x")) {
            tokens[1] = tokens[1].mid(2);
        }

        // If size is not specified, read 8 bytes
        if (tokens.size() == 2) {
            uint32_t address = tokens.at(1).toUInt(nullptr, 16);
            emit ReadSignal(address, 8);
        }
        else {
            uint32_t address = tokens.at(1).toUInt(nullptr, 16);
            // Remove "0x" from the size
            if (tokens.at(2).startsWith("0x")) {
                tokens[2] = tokens[2].mid(2);
            }
            uint16_t size = tokens.at(2).toUInt(nullptr, 16);
            emit ReadSignal(address, size);
        }
    }
    else
        UpdateLog("Unknown command: " + command, Qt::red);
}


void MainWindow::on_programBtn_clicked()
{
    emit DownloadSignal();
}


void MainWindow::on_fileLineEdit_editingFinished()
{
    QString filename = ui->fileLineEdit->text();
    emit ReadFileSignal(filename);
}

