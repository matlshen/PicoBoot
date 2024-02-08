#include "flash.h"
#include <string.h>
#include <stdbool.h>

#include "stm32f0xx.h"


bool FlashAddressValid(uint32_t address) {
    return address >= APP_START_ADDRESS && address < BL_FLASH_END_ADDRESS;
}

Boot_StatusTypeDef FlashErase(uint32_t address, uint32_t size) {
    // Check that address is within application section of Flash
    if (!FlashAddressValid(address)) {
        return BOOT_ERROR;
    }

    // Unlock flash
    int status = HAL_FLASH_Unlock();
    if (status != HAL_OK) {
        return BOOT_ERROR;
    }

    // Check that address and size are page aligned
    if ((address & (BL_FLASH_PAGE_SIZE-1)) != 0 || (size & (BL_FLASH_PAGE_SIZE-1)) != 0) {
        return BOOT_FORMAT_ERROR;
    }

    // Erase flash
    FLASH_EraseInitTypeDef erase_init = {0};
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = address & ~(BL_FLASH_PAGE_SIZE-1);
    erase_init.NbPages = (size + (BL_FLASH_PAGE_SIZE-1)) / BL_FLASH_PAGE_SIZE;

    uint32_t page_error = 0;
    status = HAL_FLASHEx_Erase(&erase_init, &page_error);
    if (status != HAL_OK) {
        return BOOT_ERROR;
    }
    if (page_error != 0xFFFFFFFF) {
        return BOOT_FLASH_ERROR;
    }

    return BOOT_OK;
}

// Boot_StatusTypeDef FlashRead(uint32_t address, void *data, uint32_t size) {
//     // Check that address is within application section of Flash
//     if (address < APP_START_ADDRESS || address + size > BL_FLASH_END_ADDRESS) {
//         return BOOT_ERROR;
//     }

//     // Copy data from Flash
//     memcpy(data, (void *)address, size);

//     return BOOT_OK;
// }

// Boot_StatusTypeDef FlashWrite(uint32_t address, const void *data, uint32_t size) {
//     // Unlock flash
//     int status = HAL_FLASH_Unlock();
//     if (status != HAL_OK) {
//         return BOOT_ERROR;
//     }

//     // Check that address is within application section of Flash
//     if (address < APP_START_ADDRESS || address + size > BL_FLASH_END_ADDRESS) {
//         return BOOT_ERROR;
//     }

//     // Check that address and size are double word aligned
//     if ((address & 0x7) != 0 || (size & 0x7) != 0) {
//         return BOOT_FORMAT_ERROR;
//     }

//     // Write data to Flash
//     for (uint32_t i = 0; i < size; i += 8) {
//         status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i, *(uint64_t*)(data+i));
//         if (status != HAL_OK) {
//             return BOOT_ERROR;
//         }
//     }

//     // Lock flash
//     HAL_FLASH_Lock();

//     return BOOT_OK;
// }