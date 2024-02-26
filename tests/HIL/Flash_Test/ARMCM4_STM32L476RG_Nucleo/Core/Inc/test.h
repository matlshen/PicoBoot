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

    uint16_t bytes_remaining = ToU16(&data[4]);

    // Receive write data in 8-byte chunks
    while (bytes_remaining > 0) {
        uint16_t bytes_to_receive = (bytes_remaining > 8) ? 8 : bytes_remaining;

        if (ComReceivePacket(&msg_id, data, &length, 100) != BOOT_OK) {
            ComNack();
            return;
        }

        // Invalid message
        if (msg_id != MSG_ID_MEM_WRITE || length != bytes_to_receive) {
            ComNack();
            return;
        }

        // If write operation was not successful, send nack
        if (FlashWrite(ToU32(&data[0]) + ToU16(&data[4]) - bytes_remaining, data, bytes_to_receive) != BOOT_OK) {
            ComNack();
            return;
        }

        bytes_remaining -= bytes_to_receive;
    }

    // Send ack if write operation was successful
    ComAck();
}

void HandleFlashRead(void) {
    // If length of message is not 6, send nack
    if (length != 6) {
        ComNack();
        return;
    }

    // If range is not valid, send nack
    if (!FlashUtil_IsRangeValid(ToU32(&data[0]), ToU16(&data[4]))) {
        ComNack();
        return;
    }

    uint16_t bytes_remaining = ToU16(&data[4]);
    uint32_t end_address = ToU32(&data[0]) + ToU16(&data[4]);

    // Read data from flash into buffer in 256 byte chunks
    while (bytes_remaining > 0) {
        uint16_t current_read_size = (bytes_remaining > 256) ? 256 : bytes_remaining;

        if (FlashRead(end_address - bytes_remaining, data, current_read_size) != BOOT_OK) {
            ComNack();
            return;
        }

        // Update bytes remaining
        bytes_remaining -= current_read_size;

        // Transmit the read data in 8-byte chunks
        while (current_read_size > 0) {
            uint16_t bytes_to_send = (current_read_size > 8) ? 8 : current_read_size;
            ComTransmitPacket(MSG_ID_MEM_READ, data, bytes_to_send);
            current_read_size -= bytes_to_send;
        }
    }

    // Send ack if read operation was successful
    ComAck();
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