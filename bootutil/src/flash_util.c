#include "flash_util.h"

bool FlashUtil_IsRangeValid(uint32_t address, uint32_t size) {
    return address >= BL_FLASH_APP_ADDRESS && address + size - 1 <= BL_FLASH_END_ADDRESS;
}

bool FlashUtil_IsPageAligned(uint32_t value) {
    return (value & (BL_FLASH_PAGE_SIZE-1)) == 0;
}

uint32_t FlashUtil_GetRangeAlignment(uint32_t address, uint32_t size) {
    uint32_t combined = address | size;
    
    return (uint32_t)(combined & -(int32_t)combined);
}