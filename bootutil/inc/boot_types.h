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
    MSG_ID_GET_CONFIG = 0x52U,
    MSG_ID_CONFIG_RESP = 0x53U,
    MSG_ID_SET_CONFIG = 0x54U,
    MSG_ID_MEM_ERASE = 0x55U,
    MSG_ID_MEM_READ = 0x56U,
    MSG_ID_MEM_READ_RESP = 0x57U,
    MSG_ID_MEM_WRITE = 0x58U,
    MSG_ID_SWAP = 0x59U,
    MSG_ID_VERIFY = 0x5AU,
    MSG_ID_GO = 0x5BU,
    MSG_ID_RESET = 0x5CU,
    MSG_ID_ACK = 0x5DU,
    MSG_ID_NACK = 0x5EU,
    MSG_ID_DATA_HTT = 0x5FU,
    MSG_ID_DATA_TTH = 0x60U,
} Boot_MsgIdTypeDef;

typedef struct {
    Boot_MsgIdTypeDef msg_id;
    uint8_t data[8];
    uint16_t length;
} Boot_CmdPacketTypeDef;

typedef struct {
    Boot_MsgIdTypeDef msg_id;
    uint8_t data[256] __attribute__((aligned(8))); /* Align to 8 bytes for flash write */
    uint16_t length;
} Boot_DataPacketTypeDef;

typedef struct {
    uint8_t major;
    uint8_t minor;
} Version_TypeDef;

/* 
 * Slot configuration section
 */
typedef struct {
    bool slot_populated;        /* Is slot populated? */
    uint32_t load_address;      /* Application start address */
    uint32_t slot_size;         /* Application slot size */
    uint32_t image_size;        /* Application image size */
    Version_TypeDef version;    /* Application image version */
    uint32_t msp;               /* MSP value for the application */
    uint32_t reset_vector;      /* Reset vector for the application */
    uint32_t hash[8];           /* SHA256 of image */
    uint32_t signature;         /* ECDSA signature of hash output */
} Slot_ConfigTypeDef;

/* Default slot config */
#define DEFAULT_SLOT_0_CONFIG { \
    .slot_populated = false, \
    .load_address = BL_SLOT_0_ADDRESS, \
    .slot_size = BL_SLOT_0_SIZE, \
    .image_size = 0, \
    .version = {0, 0}, \
    .hash = {0}, \
    .signature = 0, \
};

#define DEFAULT_SLOT_1_CONFIG { \
    .slot_populated = false, \
    .load_address = BL_SLOT_1_ADDRESS, \
    .slot_size = BL_SLOT_1_SIZE, \
    .image_size = 0, \
    .version = {0, 0}, \
    .hash = {0}, \
    .signature = 0, \
};

/* 
 * Persistent configuration section
 * The boot_config struct is stored in the configuration area of flash
 * at first available slot below the application area.
 * Make sure to doubleword alignt the struct so that it can be written into flash
 * CRC is at the beginning so the location is predictable with padding at end
 */
typedef struct __attribute__((aligned(8))) {
    uint32_t crc32;                 /* CRC32 of boot_config */
    Version_TypeDef version;         /* Bootloader version */
    uint32_t active_slot;           /* Active slot */
    Slot_ConfigTypeDef slot_list[BL_NUM_SLOTS]; /* Slot configurations */
    uint16_t node_id;                /* Node ID */
} Boot_ConfigTypeDef;

#ifdef __cplusplus
}
#endif


#endif /* BOOT_TYPES_H_ */
