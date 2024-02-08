#include "boot.h"

// Memory Configurations
#define BL_FLASH_START_ADDRESS 0x08000000U
#define BL_FLASH_PAGE_SIZE     0x800U
#define BL_FLASH_SIZE          0x20000U
#define BL_FLASH_APP_ADDRESS   0x08004000U
#define BL_FLASH_END_ADDRESS   BL_FLASH_START_ADDRESS + BL_FLASH_SIZE - 1
#define BL_SRAM_START_ADDRESS  0x20000000U
#define BL_SRAM_SIZE           0x4000U
#define BL_SRAM_END_ADDRESS    BL_SRAM_START_ADDRESS + BL_SRAM_SIZE - 1

// Eventaully this will be moved to OTP area
#define PUBLIC_KEY 0x1234567

// Static configurations
#define BL_CONFIG_ADDR  0x8001000
#define BL_NUM_SLOTS    2

// Bootloader configuration flags
#define BL_SIG_FLAG     (0x1 << 0)  // TODO: not yet supported
#define BL_ENCRYPT_FLAG (0x1 << 1)  // TODO: not yet supported
#define BL_BASH_FLAG    (0x1 << 2)  // TODO: not yet supported
#define BL_FLAG_FLAG    (0x1 << 3)  // TODO: not yet supported
#define BL_DECRYPT_FLAG (0x1 << 4)  // TODO: not yet supported
#define BL_WHAT_FLAG    (0x1 << 5)  // TODO: not yet supported
#define BL_ZOMM_FLAG    (0x1 << 6)  // TODO: not yet supported
#define BL_DEA_FLAG    (0x1 << 7)  // TODO: not yet supported

// Default slot configuration
#define BL_SLOT0_ADDR   0x08004000U
#define BL_SLOT0_SIZE   0x8000U
#define BL_SLOT1_ADDR   0x0800C000U
#define BL_SLOT1_SIZE   0x8000U