#ifndef BOOT_TYPES_H_
#define BOOT_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include "boot_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BOOT_OK = 0,
    BOOT_ERROR = -1,
    BOOT_TIMEOUT = -2,
    BOOT_FORMAT_ERROR = -3,
    BOOT_ADDRESS_ERROR = -4,
    BOOT_FLASH_ERROR = -5,
} Boot_StatusTypeDef;

typedef enum {
    MSG_ID_CONN_REQ = 0x50U,
    MSG_ID_CHANGE_SPEED = 0x51U,
    MSG_ID_CHANGE_NODE_ID = 0x52U,
    MSG_ID_GET_CONFIG = 0x53U,
    MSG_ID_MEM_ERASE = 0x58U,
    MSG_ID_MEM_WRITE = 0x59U,
    MSG_ID_MEM_READ = 0x5AU,
    MSG_ID_VERIFY = 0x5BU,
    MSG_ID_GO = 0x5CU,
    MSG_ID_RESET = 0x5DU,
    MSG_ID_ACK = 0x5EU,
    MSG_ID_NACK = 0x5FU,
} Boot_MsgIdTypeDef;

struct version {
    uint8_t major;
    uint8_t minor;
};

/* 
 * Slot configuration section
 */
typedef struct {
    bool slot_populated;        /* Is slot populated? */
    uint32_t load_address;      /* Application start address */
    uint32_t slot_size;         /* Application slot size */
    uint32_t image_size;        /* Application image size */
    struct version version;     /* Application image version */
    uint32_t hash;              /* SHA256 of image */
    uint32_t signature;         /* ECDSA signature of hash output */
} Slot_ConfigTypeDef;

/* 
 * Persistent configuration section
 * The boot_config struct is stored in the configuration area of flash
 * at address BL_CONFIG_ADDR.
 */
typedef struct {
    uint32_t active_slot;
    // Slot_ConfigTypeDef slots[BL_NUM_SLOTS]; TODO: Whyyyy
    uint32_t crc32;
} Boot_ConfigTypeDef;

#ifdef __cplusplus
}
#endif


#endif /* BOOT_TYPES_H_ */
