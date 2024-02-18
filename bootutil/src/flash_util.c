#include "flash_util.h"

/**
 * @brief Check whether specified range is within application section of Flash.
 * @param address Start address of range.
 * @param size Number of bytes in range.
 * @return true if range is valid, false otherwise.
*/
bool FlashUtil_IsRangeValid(uint32_t address, uint32_t size) {
    return address >= BL_APP_START_ADDRESS && address + size - 1 <= BL_FLASH_END_ADDRESS;
}

/**
 * @brief Check whether specified value is page-aligned.
 * @param value Value to check. Can be address or size.
 * @return true if value is page-aligned, false otherwise.
*/
bool FlashUtil_IsPageAligned(uint32_t value) {
    return (value & (BL_FLASH_PAGE_SIZE-1)) == 0;
}

/**
 * @brief Get alignment of range.
 * @param address Start address of range.
 * @param size Number of bytes in range.
 * @return Least significant bit set in both address and size.
*/
uint32_t FlashUtil_GetRangeAlignment(uint32_t address, uint32_t size) {
    uint32_t combined = address | size;
    
    return (uint32_t)(combined & -(int32_t)combined);
}