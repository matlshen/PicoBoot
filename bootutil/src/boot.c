#include "boot.h"
#include "com.h"

/* State machine definitions */
typedef enum {
    INIT,
    WAITING_FOR_CONNECTION,
    WAITING_FOR_COMMAND,
    VERIFY,
    HANDLE_TIMEOUT,
} Boot_StateTypeDef;
static Boot_StateTypeDef boot_state = WAITING_FOR_COMMAND;
static bool in_bootloader_mode = false;
static Boot_StatusTypeDef status;

void BootStateMachine(void) {
    switch(boot_state) {
        case INIT:
            Init();
            boot_state = WAITING_FOR_CONNECTION;
            break;
        case WAITING_FOR_CONNECTION:
            WaitForConnection();
            break;
        case WAITING_FOR_COMMAND:
            WaitForCommand();
            break;
        case HANDLE_TIMEOUT:
            HandleTimeout();
            break;
        default:
            // Shouldn't get here
            boot_state = INIT;
            break;
    }
}

/* Packet buffer */
static Boot_MsgIdTypeDef msg_id;
static uint8_t data[256];
static uint8_t length;

// Ignore this if running emulator
#ifdef TARGET
void Init(void) {
    // Initialize communication interface
    ComInit();

    // Check validity of boot_config
    Boot_ConfigTypeDef *p_cfg = (Boot_ConfigTypeDef *)BL_CONFIG_ADDR;
    uint32_t calculated_crc = crc32((uint8_t *)p_cfg, 
                                    sizeof(Boot_ConfigTypeDef),
                                    INITIAL_CRC);

    // If boot_config is invalid, store default boot_config in Flash
    if (calculated_crc != p_cfg->crc32) {
        // Store default boot_config in Flash
        // slot_configs are all initialized to 0
        Boot_ConfigTypeDef default_cfg = {0};
        default_cfg.crc32 = crc32((uint8_t *)&default_cfg, 
                                 sizeof(Boot_ConfigTypeDef),
                                 INITIAL_CRC);
                                 
        FlashErase(BL_CONFIG_ADDR, sizeof(Boot_ConfigTypeDef));
        FlashWrite(BL_CONFIG_ADDR, &default_cfg, sizeof(Boot_ConfigTypeDef));
    }
}
#else
void Init(void) {
    // Initialize communication interface
    ComInit();
}
#endif

void WaitForConnection(void) {
    // Wait for connection
    status = ComReceivePacket(&msg_id, data, &length, BL_TIMEOUT_MS);

    // If connection request, ack and go to waiting for command
    if (status == BOOT_OK && msg_id == MSG_ID_CONN_REQ) {
        // TODO: Implement node id check
        ComAck();
        boot_state = WAITING_FOR_COMMAND;
    }

    // If host timed out
    else if (status == BOOT_TIMEOUT)
        boot_state = HANDLE_TIMEOUT;

    // If other problem, nack and continue waiting for connection
    else
        ComNack();
}

void WaitForCommand(void) {
    // Wait for command
    status = ComReceivePacket(&msg_id, data, &length, BL_COMMAND_TIMEOUT_MS);

    if (status == BOOT_OK) {
        // If command, handle command and go back to waiting for command
        switch (msg_id) {
            case MSG_ID_CHANGE_SPEED:
                // TODO: Implement this
                ChangeSpeed();
                break;
            case MSG_ID_CHANGE_NODE_ID:
                // TODO: Implement this
                ChangeNodeId();
                break;
            case MSG_ID_GET_CONFIG:
                // TODO: Implement this
                GetConfig();
                break;
            case MSG_ID_MEM_ERASE:
                EraseMemory();
                break;
            case MSG_ID_MEM_WRITE:
                WriteMemory();
                break;
            case MSG_ID_MEM_READ:
                ReadMemory();
                break;
            case MSG_ID_VERIFY:
                // TODO: Implement this
                Verify();
                break;
            case MSG_ID_GO:
                // TODO: Implement this
                break;
            case MSG_ID_RESET:
                // Directly call low-level function
                // Reset can't return or error
                SystemReset();
                break;
            default:
                // Invalid command
                ComNack();
                break;
        }
    }

    // Timeout
    else if (status == BOOT_TIMEOUT)
        boot_state = HANDLE_TIMEOUT;
    // Other problem
    else
        ComNack();
}

void ChangeSpeed(void) {
    // TODO: Implement this
    Boot_MsgIdTypeDef tx_msg_id = MSG_ID_CHANGE_SPEED;
    uint8_t tx_data[] = {0x92, 0x82, 0x71, 0x69};
    uint8_t tx_length = 4;
    ComTransmitPacket(tx_msg_id, tx_data, tx_length);
    ComNack();
} 

void ChangeNodeId(void) {
    // TODO: Implement this
    Boot_MsgIdTypeDef tx_msg_id = MSG_ID_CHANGE_NODE_ID;
    uint8_t tx_data[] = {0x11, 0x12, 0x13, 0x14};
    uint8_t tx_length = 4;
    ComTransmitPacket(tx_msg_id, tx_data, tx_length);
    ComNack();
}

void GetConfig(void) {
    // TODO: Implement this
    ComNack();
}

void EraseMemory(void) {
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

void WriteMemory(void) {
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

    // Receive write data in 256 byte chunks
    while (bytes_remaining > 0) {
        uint16_t bytes_to_receive = (bytes_remaining > 256) ? 256 : bytes_remaining;

        // Receive chunk data
        if (ComReceive(data, bytes_to_receive, 100) != BOOT_OK) {
            ComNack();
            return;
        }

        bytes_remaining -= bytes_to_receive;

        // If 256 bytes have been received or this is last chunk,
        // receive the checksum and verify
        if (bytes_to_receive == 256 || bytes_remaining == 0) {
            uint32_t received_checksum;
            if (ComReceive((uint8_t*)&received_checksum, 4, 100) != BOOT_OK) {
                ComNack();
                return;
            }

            uint32_t calculated_checksum = crc32(data, bytes_to_receive, INITIAL_CRC);

            // If checksums do not match, send nack
            // Otherwise, write to flash and send ack
            if (received_checksum != calculated_checksum) {
                ComNack();
                return;
            } else {
                // TODO: Verify that this is correct
                if (FlashWrite(end_address - bytes_remaining - bytes_to_receive, data, bytes_to_receive) != BOOT_OK) {
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

void ReadMemory(void) {
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

    // Read data from flash into buffer in 256 byte chunks
    while (bytes_remaining > 0) {
        uint16_t current_read_size = (bytes_remaining > 256) ? 256 : bytes_remaining;

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

void Verify(void) {
    // TODO: Implement this
    Boot_MsgIdTypeDef tx_msg_id = MSG_ID_VERIFY;
    uint8_t tx_data[] = {0x55, 0x66, 0x77, 0x88};
    uint8_t tx_length = 4;
    ComTransmitPacket(tx_msg_id, tx_data, tx_length);
    ComNack();
}

void Go(void) {
    // TODO: Implement this
    ComNack();
}

void HandleTimeout(void) {
    // If in bootloader mode, do nothing
    if (in_bootloader_mode) {
        boot_state = WAITING_FOR_COMMAND;
        return;
    }

    // If not in bootloader mode, verify application
    Verify();

    // If application is valid, go to application
    if (status == BOOT_OK)
        Go();
    else {
        // If application is invalid, enter bootloader mode
        in_bootloader_mode = true;
    }
}