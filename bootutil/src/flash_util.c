#include "flash_util.h"
#include "boot.h"
#include <memory.h>

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
 * @brief Convert 32-bit unsigned integer to byte array.
 * @param value 32-bit unsigned integer.
 * @param data Byte array.
*/
void U32ToBytes(uint32_t value, uint8_t *data) {
    memcpy(data, &value, sizeof(value));
}

/**
 * @brief Convert 16-bit unsigned integer to byte array.
 * @param value 16-bit unsigned integer.
 * @param data Byte array.
*/
void U16ToBytes(uint16_t value, uint8_t *data) {
    memcpy(data, &value, sizeof(value));
}

/**
 * @brief Convert address and size to byte array.
 * @param address Address.
 * @param size Size.
 * @param data Byte array.
*/
void ToFlashPacket(uint32_t address, uint16_t size, uint8_t *data) {
    U32ToBytes(address, data);
    U16ToBytes(size, data + 4);
}

/**
 * @brief Convert byte array to address and size.
 * @param address Address.
 * @param size Size.
 * @param data Byte array.
*/
void FromFlashPacket(uint32_t *address, uint16_t *size, const uint8_t *data) {
    *address = ToU32(data);
    *size = ToU16(data + 4);
}

/**
 * @brief Check whether specified range is within application section of Flash.
 * @param address Start address of range.
 * @param size Number of bytes in range.
 * @return true if range is valid, false otherwise.
*/
bool FlashUtil_IsRangeValid(uint32_t address, uint32_t size) {
    // Check whether application is contained in valid slot
    #ifdef TARGET
    for (uint32_t i = 0; i < BL_NUM_SLOTS; i++) {
        if (address >= p_cfg->slot_list[i].load_address &&
            address + size <= p_cfg->slot_list[i].load_address + p_cfg->slot_list[i].slot_size) {
            return true;
        }
    }
    #endif
    return false;
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
 * @brief Round specified value up to next page boundary.
 * @param value Value to round.
 * @return Rounded page-aligned value.
*/
uint32_t FlashUtil_RoundUpToPage(uint32_t value) {
    return (value + BL_FLASH_PAGE_SIZE - 1) & ~(BL_FLASH_PAGE_SIZE - 1);
}

/**
 * @brief Round specified value down to next page boundary.
 * @param value Value to round.
 * @return Rounded page-aligned value.
*/
uint32_t FlashUtil_RoundDownToPage(uint32_t value) {
    return value & ~(BL_FLASH_PAGE_SIZE - 1);
}

/**
 * @brief Get page number for specified address.
 * @param address Address to get page number for.
 * @return Page number.
*/
uint32_t FlashUtil_GetPageNum(uint32_t address) {
    return (address - BL_FLASH_START_ADDRESS) / BL_FLASH_PAGE_SIZE;
}

/**
 * @brief Get number of pages required to store specified number of bytes.
 * @param size Number of bytes.
 * @return Number of pages.
*/
uint32_t FlashUtil_GetNumPages(uint32_t size) {
    return (size + BL_FLASH_PAGE_SIZE - 1) / BL_FLASH_PAGE_SIZE;
}

/**
 * @brief Get bank number for specified address.
 * @param address Address to get bank number for.
 * @return Bank number.
*/
uint32_t FlashUtil_GetBankNum(uint32_t address) {
    return FlashUtil_GetPageNum(address) / BL_FLASH_BANK_SIZE + 1;
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
