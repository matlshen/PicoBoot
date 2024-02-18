#include "boot.h"

typedef enum {
    INIT,
    WAITING_FOR_CONNECTION
} Boot_StateTypeDef;
static Boot_StateTypeDef boot_state = INIT;
static Boot_StateTypeDef status;

void BootStateMachine(void) {
    switch(boot_state) {
        case INIT:
            BootInit();
            boot_state = WAITING_FOR_CONNECTION;
            break;
        case WAITING_FOR_CONNECTION:
            // Wait for connection
            break;
    }
}

void Init(void) {
    // Check validity of boot_config
    struct boot_config *p_cfg = (struct boot_config *)BL_CONFIG_ADDR;
    uint32_t calculated_crc = crc32((uint8_t *)p_cfg, 
                                    sizeof(struct boot_config), 
                                    INITIAL_CRC);

    // If boot_config is invalid, store default boot_config in Flash
    if (calculated_crc != p_cfg->crc32) {
        // Store default boot_config in Flash
        // slot_configs are all initialized to 0
        struct boot_config default_cfg = {0};
        default_cfg.crc32 = crc32((uint8_t *)&default_cfg, 
                                 sizeof(struct boot_config), 
                                 INITIAL_CRC);
                                 
        FlashErase(BL_CONFIG_ADDR, sizeof(struct boot_config));
        FlashWrite(BL_CONFIG_ADDR, &default_cfg, sizeof(struct boot_config));
    }
}

void WaitForConnection(void) {
    // Wait for connection
    // If connection is established, change state to UPDATE
    // If timeout occurs, change state to INIT
}