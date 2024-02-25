#ifndef TEST_H_
#define TEST_H_

#include "com.h"

// Data buffer
Boot_MsgIdTypeDef msg_id;
uint8_t data[256];
uint8_t length;
uint8_t error_sequence[6] = "error";

void TestFunction(void) {
    ComInit();

    while (1) {
        if (ComReceivePacket(&msg_id, data, &length, 50) == BOOT_OK) {
            // Transmit ACK
            ComAck();

            // Transmit the received data back over UART
            ComTransmitPacket(msg_id, data, length);
        }
        else {
            // Transmit NACK
            ComNack();
        }
    }
}


#endif /* TEST_H_ */