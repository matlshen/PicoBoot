#ifndef TEST_H_
#define TEST_H_

#include "com.h"

// Data buffer
Boot_StatusTypeDef status;
Boot_MsgIdTypeDef msg_id;
uint8_t data[256];
uint8_t length;
uint8_t error_sequence[6] = "error";

void TestFunction(void) {
    ComInit();

    while (1) {
        status = ComReceivePacket(&msg_id, data, &length, 50);

        // Echo packet if received successfully
        if (status == BOOT_OK) {
            ComTransmitPacket(msg_id, data, length);
        }
        // Transmit ACK continuously on timeout
        else if (status == BOOT_TIMEOUT) {
            ComAck();
        }
        // Transmit NACK if data format error
        else {
            ComNack();
        }
    }
}


#endif /* TEST_H_ */