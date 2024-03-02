#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void UpdateLog(QString msg, const QBrush& color = Qt::black);
    void UpdateProgress(int progress);

signals:
    void ConnectSignal(QString portName);
    void GetConfigSignal();
    void EraseSignal(uint32_t address, uint16_t size);
    void ReadSignal(uint32_t address, uint16_t size);
    void ReadFileSignal(QString filename);
    void DownloadSignal();

private slots:
    void on_browseBtn_clicked();
    void on_connectBtn_clicked();
    void on_lineEdit_returnPressed();

    void on_programBtn_clicked();

    void on_fileLineEdit_editingFinished();

private:
    Ui::MainWindow *ui;
    void LoadPorts();
};
#endif // MAINWINDOW_H
