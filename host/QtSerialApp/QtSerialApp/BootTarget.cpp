#include "BootTarget.h"
#include <stdint>

BootTarget::BootTarget(QObject *parent)
    : QObject{parent}
{}

bool BootTarget::Connect()
{
    // Send conenction reqest
    QByteArray message;
}
