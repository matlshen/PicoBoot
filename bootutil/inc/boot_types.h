#include "boot.h"

typedef enum {
    BOOT_OK = 0,
    BOOT_ERROR = -1,
    BOOT_TIMEOUT = -2,
    BOOT_FORMAT_ERROR = -3,
    BOOT_ADDRESS_ERROR = -4,
    BOOT_FLASH_ERROR = -5,
} Boot_StatusTypeDef;


struct version {
    uint8_t major;
    uint8_t minor;
};

/* 
 * Slot configuration section
 */
struct slot_config {
    bool slot_populated;        /* Is slot populated? */
    uint32_t load_address;      /* Application start address */
    uint32_t slot_size;         /* Application slot size */
    uint32_t image_size;        /* Application image size */
    struct version version;     /* Application image version */
    uint32_t hash;              /* SHA256 of image */
    uint32_t signature;         /* ECDSA signature of hash output */
};

/* 
 * Persistent configuration section
 * The boot_config struct is stored in the configuration area of flash
 * at address BL_CONFIG_ADDR.
 */
struct boot_config {
    uint32_t active_slot;
    struct slot_config slots[BL_NUM_SLOTS];
    uint32_t crc32;
};