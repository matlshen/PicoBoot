#include "boot.h"

void BootInit(void) {
    // Check validity of boot_config
    struct boot_config *p_cfg = (struct boot_config *)BOOT_CONFIG_ADDR;
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
                                 
        FlashErase(BOOT_CONFIG_ADDR, sizeof(struct boot_config));
        FlashWrite(BOOT_CONFIG_ADDR, &default_cfg, sizeof(struct boot_config));
    }
}