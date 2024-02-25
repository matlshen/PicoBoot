#ifndef TEST_H_
#define TEST_H_

#include "com.h"
#include "flash.h"
#include "flash_util.h"

// Data buffer
Boot_StatusTypeDef status;
Boot_MsgIdTypeDef msg_id;
uint8_t data[256];
uint8_t length;
uint8_t error_sequence[6] = "error";

void HandleFlashErase(void) {
    // If length of received packet is not 6, send nack and return
    if (length != 6) {
        ComNack();
        return;
    }

    // If erase operation was not successful, send nack
    if (FlashErase(ToU32(&data[0]), ToU16(&data[4])) != BOOT_OK) {
        ComNack();
        return;
    }
    
    // If erase operation was successful, send ack
    ComAck();
}

void HandleFlashWrite(void) {
    // If length of received packet is not 6, send nack and return
    if (length != 6) {
        ComNack();
        return;
    }

    // If range is not valid, send nack
    if (!FlashUtil_IsRangeValid(ToU32(&data[0]), ToU16(&data[4]))) {
        ComNack();
        return;
    }

    // Receive write data in 8-byte chunks
    for (uint32_t i = 0; i < ToU16(&data[4]); i += 8) {
        if (ComReceivePacket(&msg_id, data, &length, 100) != BOOT_OK) {
            ComNack();
            return;
        }

        // Invalid message
        if (msg_id != MSG_ID_MEM_WRITE || length != 8) {
            ComNack();
            return;
        }

        FlashWrite(ToU32(&data[0]) + i, data, 8);
    }

    // Receive remainder of data
    uint16_t remainder = ToU16(&data[4]) % 8;
    if (remainder > 0) {
        if (ComReceivePacket(&msg_id, data, &length, 100) != BOOT_OK) {
            ComNack();
            return;
        }

        // Number of bytes does not align
        if (msg_id != MSG_ID_MEM_WRITE || length != remainder) {
            ComNack();
            return;
        }

        FlashWrite(ToU32(&data[0]) + ToU16(&data[4]) - remainder, data, remainder);
    }

    // Send ack if write operation was successful
    ComAck();
}

void HandleFlashRead(void) {
    // Read data from flash into buffer
    status = FlashRead(ToU32(&data[0]), data, ToU16(&data[4]));

    // Send read data in 8-byte chunks
    for (uint32_t i = 0; i < ToU16(&data[4]); i += 8) {
        ComTransmitPacket(MSG_ID_MEM_READ, &data[i], 8);
    }

    // Send remainder of data
    uint16_t remainder = ToU16(&data[4]) % 8;
    if (remainder > 0) {
        ComTransmitPacket(MSG_ID_MEM_READ, &data[ToU16(&data[4]) - remainder], remainder);
    }
}

void HandleFlashRead(void) {
    status = FlashRead(data[0], data[1], &data[2], length - 2);
}

void TestFunction(void) {
    ComInit();

    while (1) {
        status = ComReceivePacket(&msg_id, data, &length, 5000);

        // If packet was received successfully
        if (status == BOOT_OK) {
            switch(msg_id) {
                case MSG_ID_MEM_ERASE:
                    HandleFlashErase();
                    break;
                case MSG_ID_MEM_WRITE:
                    HandleFlashWrite();
                    break;
                case MSG_ID_MEM_READ:
                    HandleFlashRead();
                    break;
                default:
                    status = BOOT_ERROR;
                    break;
            }
        }

        // Send nack if packet was not received successfully
        else {
            ComNack();
        }
    }
}


#endif /* TEST_H_ */