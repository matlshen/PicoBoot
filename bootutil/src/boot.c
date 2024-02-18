#include "boot.h"
#include "com.h"

/* State machine definitions */
typedef enum {
    INIT,
    WAITING_FOR_CONNECTION,
    WAITING_FOR_COMMAND,
    VERIFY,
} Boot_StateTypeDef;
static Boot_StateTypeDef boot_state = INIT;
static Boot_StateTypeDef status;

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
    }
}

/* Packet buffer */
static Boot_MsgIdTypeDef msg_id;
static uint8_t data[256];
static uint8_t length;

// Ignore this if running on host
#ifdef TARGET
void Init(void) {
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
void Init(void) {}
#endif

void WaitForConnection(void) {
    // Wait for connection
    status = ComReceivePacket(&msg_id, data, &length, BL_TIMEOUT_MS);

    // If timeout, verify application
    if (status == BOOT_TIMEOUT) {
        boot_state = VERIFY;
    }

    // If connection request, ack and go to waiting for command
    else if (msg_id == MSG_ID_CONN_REQ) {
        ComAck();
        boot_state = WAITING_FOR_COMMAND;
    }

    // If other problem, nack
    else {
        ComNack();
    }
}

void WaitForCommand(void) {
    // Wait for command
    status = ComReceivePacket(&msg_id, data, &length, BL_COMMAND_TIMEOUT_MS);

    // If timeout, verify application
    if (status == BOOT_TIMEOUT) {
        boot_state = VERIFY;
    }
}
