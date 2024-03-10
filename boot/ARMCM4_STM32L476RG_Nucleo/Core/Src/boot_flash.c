#include "flash.h"
#include "flash_util.h"
#include <string.h>
#include <stdbool.h>

#include "stm32l4xx_hal.h"

/**
 * @brief Check if address and size are valid for erase operation. Range must be 
 * within application section of Flash and page-aligned.
 * @param address Start address to erase. 
 * @param size Number of bytes to erase. 
 * @return True if range is valid, otherwise false.
*/
bool FlashEraseRangeValid(uint32_t address, uint32_t size) {
    // Check that address is within application section of Flash
    if (!FlashUtil_IsRangeValid(address, size)) {
        return false;
    }

    // Check that address and size are page aligned
    if (!FlashUtil_IsPageAligned(address) || !FlashUtil_IsPageAligned(size)) {
        return false;
    }

    return true;
}

/**
 * @brief Check if address and size are valid for read operation. Range must be
 * within application section of Flash.
 * @param address Start address to read.
 * @param size Number of bytes to read. 
 * @return True if range is valid, otherwise false.
*/
bool FlashReadRangeValid(uint32_t address, uint32_t size) {
    return FlashUtil_IsRangeValid(address, size);
}

/**
 * @brief Check if address and size are valid for write operation. Range must be
 * within application section of Flash and at least doubleword-aligned.
 * @param address Start address to write.
 * @param size Number of bytes to write.
 * @return True if range is valid, otherwise false.
*/
bool FlashWriteRangeValid(uint32_t address, uint32_t size) {
    // Check that address is within application section of Flash
    if (!FlashUtil_IsRangeValid(address, size)) {
        return false;
    }

    // Check that address and size are at least doubleword aligned
    if (FlashUtil_GetRangeAlignment(address, size) % 0x8 != 0){
        return false;
    }

    return true;
}

/**
 * @brief Erase region of Flash memory.
 * @param address Start address to erase. Must be page-aligned.
 * @param size Number of bytes to erase. Must be page-aligned.
 * @return BOOT_OK if successful, otherwise error code.
*/
Boot_StatusTypeDef FlashErase(uint32_t address, uint32_t size) {
    // Check that address and size are valid
    if (!FlashEraseRangeValid(address, size)) {
        return BOOT_ADDRESS_ERROR;
    }

    return BootFlashErase(address, size);
}

/**
 * @brief Read data from Flash memory.
 * @param address Start address to read from. Must be within application section of Flash.
 * @param data Buffer to read data into.
 * @param size Number of bytes to read.
 * @return BOOT_OK if successful, otherwise error code.
*/
Boot_StatusTypeDef FlashRead(uint32_t address, void *data, uint32_t size) {
    // Check that range is valid
    if (!FlashReadRangeValid(address, size)) {
        return BOOT_ADDRESS_ERROR;
    }

    return BootFlashRead(address, data, size);
}

/**
 * @brief Write data to Flash memory.
 * @param address Start address to write to. Must be within application section of Flash. Must be at least halfword-aligned.
 * @param data Buffer to write data from.
 * @param size Number of bytes to write. Must be at least halfword-aligned.
 * @return BOOT_OK if successful, otherwise error code.
*/
Boot_StatusTypeDef FlashWrite(uint32_t address, const void *data, uint32_t size) {
    // Check that address is within application section of Flash
    if (!FlashUtil_IsRangeValid(address, size)) {
        return BOOT_ADDRESS_ERROR;
    }

    return BootFlashWrite(address, data, size);
}

/**
 * @brief Unrestricted erase region of Flash memory.
 * @param address Start address to erase. Must be page-aligned.
 * @param size Number of bytes to erase. Must be page-aligned.
 * @return BOOT_OK if successful, otherwise error code.
*/
Boot_StatusTypeDef BootFlashErase(uint32_t address, uint32_t size) {
    // Unlock flash
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return BOOT_ERROR;
    }

    // Erase flash
    FLASH_EraseInitTypeDef erase_init = {0};
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.Banks = FlashUtil_GetBankNum(address);
    erase_init.Page = FlashUtil_GetPageNum(address);
    erase_init.NbPages = FlashUtil_GetNumPages(size);

    uint32_t page_error = 0;
    if (HAL_FLASHEx_Erase(&erase_init, &page_error) != HAL_OK) {
        return BOOT_ERROR;
    }
    if (page_error != 0xFFFFFFFF) {
        return BOOT_FLASH_ERROR;
    }

    // Lock flash
    HAL_FLASH_Lock();

    return BOOT_OK;
}

/**
 * @brief Unrestricted read data from Flash memory.
 * @param address Start address to read from. 
 * @param data Buffer to read data into.
 * @param size Number of bytes to read.
 * @return BOOT_OK if successful, otherwise error code.
*/
Boot_StatusTypeDef BootFlashRead(uint32_t address, void *data, uint32_t size) {
    // Copy data from Flash
    memcpy(data, (void *)address, size);

    return BOOT_OK;
}

/**
 * @brief Unrestricted write to flash memory.
 * @param address Start address to write to. Must be at least doubleword-aligned.
 * @param data Buffer to write data from.
 * @param size Number of bytes to write. Must be at least doubleword-aligned.
 * @return BOOT_OK if successful, otherwise error code.
*/
Boot_StatusTypeDef BootFlashWrite(uint32_t address, const void *data, uint32_t size) {
    // Unlock flash
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return BOOT_ERROR;
    }

    // Write data to Flash
    for (uint32_t i = 0; i < size; i += 8) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i, *(uint64_t*)(data+i)) != HAL_OK) {
            return BOOT_ERROR;
        }
    }

    // Lock flash
    HAL_FLASH_Lock();

    return BOOT_OK;
}