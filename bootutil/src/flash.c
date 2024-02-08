#include "flash.h"
#include <string.h>
#include <stdbool.h>

#include "stm32f0xx.h"

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
    erase_init.PageAddress = address & ~(BL_FLASH_PAGE_SIZE-1);
    erase_init.NbPages = (size + (BL_FLASH_PAGE_SIZE-1)) / BL_FLASH_PAGE_SIZE;

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

Boot_StatusTypeDef FlashRead(uint32_t address, void *data, uint32_t size) {
    // Check that range is valid
    if (!FlashUtil_IsRangeValid(address, size)) {
        return BOOT_ADDRESS_ERROR;
    }

    // Copy data from Flash
    memcpy(data, (void *)address, size);

    return BOOT_OK;
}

Boot_StatusTypeDef FlashWrite(uint32_t address, const void *data, uint32_t size) {
    // Check that address is within application section of Flash
    if (!FlashUtil_IsRangeValid(address, size)) {
        return BOOT_ADDRESS_ERROR;
    }

    // Check that address and size are double word aligned
    if ((address & 0x7) != 0 || (size & 0x7) != 0) {
        return BOOT_ADDRESS_ERROR;
    }

    // Get minimum alignmnet of address and size
    uint32_t alignment = FlashUtil_GetRangeAlignment(address, size);
    uint32_t type_program;
    switch (alignment) {
        case 0x2:
            type_program = FLASH_TYPEPROGRAM_HALFWORD;
            break;
        case 0x4:
            type_program = FLASH_TYPEPROGRAM_WORD;
            break;
        case 0x8:
            type_program = FLASH_TYPEPROGRAM_DOUBLEWORD;
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