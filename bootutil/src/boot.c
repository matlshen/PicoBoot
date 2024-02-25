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
static Boot_StateTypeDef boot_state = INIT;
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
                ChangeSpeed();
                break;
            case MSG_ID_CHANGE_NODE_ID:
                ChangeNodeId();
                break;
            case MSG_ID_GET_CONFIG:
                GetConfiguration();
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
                Verify();
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
    // TODO: Implement this
    ComNack();
}

void WriteMemory(void) {
    // TODO: Implement this
    ComNack();
}

void ReadMemory(void) {
    // TODO: Implement this
    ComNack();
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
    if (in_bootloader_mode)
        return;

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