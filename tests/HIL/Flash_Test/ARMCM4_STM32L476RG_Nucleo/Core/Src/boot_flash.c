#include "flash.h"
#include <string.h>
#include <stdbool.h>

#include "stm32l4xx.h"

/**
 * @brief Erase region of Flash memory.
 * @param address Start address to erase. Must be page-aligned.
 * @param size Number of bytes to erase. Must be page-aligned.
 * @return BOOT_OK if successful, otherwise error code.
*/
Boot_StatusTypeDef FlashErase(uint32_t address, uint32_t size) {
    // Check that address is within application section of Flash
    if (!FlashUtil_IsRangeValid(address, size)) {
        return BOOT_ADDRESS_ERROR;
    }

    // Check that address and size are page aligned
    if (!FlashUtil_IsPageAligned(address) || !FlashUtil_IsPageAligned(size)) {
        return BOOT_ADDRESS_ERROR;
    }

    // Unlock flash
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return BOOT_ERROR;
    }

    // Erase flash
    FLASH_EraseInitTypeDef erase_init = {0};
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.Banks = FLASH_BANK_1;
    erase_init.Page = FlashUtil_GetPage(address);
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
 * @brief Read data from Flash memory.
 * @param address Start address to read from. Must be within application section of Flash.
 * @param data Buffer to read data into.
 * @param size Number of bytes to read.
 * @return BOOT_OK if successful, otherwise error code.
*/
Boot_StatusTypeDef FlashRead(uint32_t address, void *data, uint32_t size) {
    // Check that range is valid
    if (!FlashUtil_IsRangeValid(address, size)) {
        return BOOT_ADDRESS_ERROR;
    }

    // Copy data from Flash
    memcpy(data, (void *)address, size);

    return BOOT_OK;
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

    // Get minimum alignmnet of address and size
    uint32_t alignment = FlashUtil_GetRangeAlignment(address, size);
    uint32_t type_program;
    switch (alignment) {
        case 0x8:
            type_program = FLASH_TYPEPROGRAM_DOUBLEWORD;
            break;
        case 0x100:
            type_program = FLASH_TYPEPROGRAM_FAST;
            break;
        default:
            return BOOT_ADDRESS_ERROR;
    }

    // Unlock flash
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return BOOT_ERROR;
    }

    // Write data to Flash
    for (uint32_t i = 0; i < size; i += alignment) {
        if (HAL_FLASH_Program(type_program, address + i, *(uint64_t*)(data+i)) != HAL_OK) {
            return BOOT_ERROR;
        }
    }

    // Lock flash
    HAL_FLASH_Lock();

    return BOOT_OK;
}