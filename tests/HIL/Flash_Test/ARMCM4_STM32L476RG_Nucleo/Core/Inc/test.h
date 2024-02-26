#ifndef TEST_H_
#define TEST_H_

#include "com.h"
#include "flash.h"
#include "flash_util.h"
#include "crc32.h"

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

    uint32_t start_address = ToU32(&data[0]);
    uint16_t bytes_remaining = ToU16(&data[4]);
    uint32_t end_address = ToU32(&data[0]) + bytes_remaining;

    // If write range is not valid, send nack and return
    // Otherwise, send ack to signal ready to receive write data
    if (!FlashWriteRangeValid(start_address, bytes_remaining)) {
        ComNack();
        return;
    } else {
        ComAck();
    }

    // Receive write data in 255 byte chunks
    while (bytes_remaining > 0) {
        uint16_t bytes_to_receive = (bytes_remaining > 255) ? 255 : bytes_remaining;

        // Receive chunk data
        if (ComReceive(data, bytes_to_receive, 100) != BOOT_OK) {
            ComNack();
            return;
        }

        bytes_remaining -= bytes_to_receive;

        // If 255 bytes have been received or this is last chunk,
        // receive the checksum and verify
        if (bytes_to_receive == 255 || bytes_remaining == 0) {
            if (ComReceive(data, 4, 100) != BOOT_OK) {
                ComNack();
                return;
            }

            uint32_t checksum = ToU32(data);
            uint32_t calculated_checksum = crc32(data, 255, INITIAL_CRC);

            // If checksums do not match, send nack
            // Otherwise, write to flash and send ack
            if (checksum != calculated_checksum) {
                ComNack();
                return;
            } else {
                // TODO: Verify that this is correct
                if (FlashWrite(end_address - bytes_remaining - 255, data, 255) != BOOT_OK) {
                    ComNack();
                    return;
                }
                ComAck();
            }
        }
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

    uint32_t start_address = ToU32(&data[0]);
    uint16_t bytes_remaining = ToU16(&data[4]);
    uint32_t end_address = ToU32(&data[0]) + bytes_remaining;

    // If read range is not valid, send nack and return
    // Otherwise, send ack to signal start of read data
    if (!FlashReadRangeValid(start_address, bytes_remaining)) {
        ComNack();
        return;
    } else {
        ComAck();
    }

    // Read data from flash into buffer in 255 byte chunks
    while (bytes_remaining > 0) {
        uint8_t current_read_size = (bytes_remaining > 255) ? 255 : bytes_remaining;

        if (FlashRead(end_address - bytes_remaining, data, current_read_size) != BOOT_OK) {
            ComNack();
            return;
        }

        // Update bytes remaining
        bytes_remaining -= current_read_size;

        // Transmit the current chunk
        if (ComTransmit(data, current_read_size, 100) != BOOT_OK) {
            ComNack();
            return;
        }
    }

    // Send ack to signal end of read data
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