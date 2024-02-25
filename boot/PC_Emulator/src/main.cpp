#include <QCoreApplication>
#include "boot.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    while (1) {
        BootStateMachine();
    }


    return a.exec();
}
