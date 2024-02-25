#include <QCoreApplication>
#include <QElapsedTimer>
#include "com.h"

/* Wait for consecutive NACKs to be received */
void Test1() {
    QElapsedTimer timer;
    Boot_MsgIdTypeDef msg_id;
    uint8_t data[256];
    uint8_t length;

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    timer.start();

    if (msg_id != MSG_ID_NACK) {
        qDebug() << "Test1 Error: Expected NACK 1, got" << msg_id;
        return;
    }

    ComReceivePacket((Boot_MsgIdTypeDef*)&msg_id, data, &length, 100);
    qint64 elapsed = timer.elapsed();

    if (msg_id != MSG_ID_NACK) {
        qDebug() << "Test1 Error: Expected NACK 2, got" << msg_id;
        return;
    }

    if (elapsed < 50*0.9) {
        qDebug() << "Test1 Error: Target timeout too short, " << elapsed << "ms";
        return;
    }

    if (elapsed > 50*1.1) {
        qDebug() << "Test1 Error: Target timeout too long, " << elapsed << "ms";
        return;
    }

    qDebug() << "Test1: Passed";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ComInit();

    Test1();

    return a.exec();
}
