#include "flash_util.h"

/**
 * @brief Convert byte array to 32-bit unsigned integer.
 * @param data Byte array.
 * @return Converted 32-bit unsigned integer.
*/
uint32_t ToU32(const uint8_t *data) {
    return (uint32_t)data[0] | (uint32_t)data[1] << 8 | (uint32_t)data[2] << 16 | (uint32_t)data[3] << 24;
}

/**
 * @brief Convert byte array to 16-bit unsigned integer.
 * @param data Byte array.
 * @return Converted 16-bit unsigned integer.
*/
uint16_t ToU16(const uint8_t *data) {
    return (uint16_t)data[0] | (uint16_t)data[1] << 8;
}

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