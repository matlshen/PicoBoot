#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>

#include "boot.h"

#ifdef __cplusplus
extern "C" {
#endif

bool FlashEraseRangeValid(uint32_t address, uint32_t size);
bool FlashReadRangeValid(uint32_t address, uint32_t size);
bool FlashWriteRangeValid(uint32_t address, uint32_t size);

Boot_StatusTypeDef FlashErase(uint32_t address, uint32_t size);
Boot_StatusTypeDef FlashRead(uint32_t address, void *data, uint32_t size);
Boot_StatusTypeDef FlashWrite(uint32_t address, const void *data, uint32_t size);

// TODO: Better way to expose these functions?
Boot_StatusTypeDef BootFlashErase(uint32_t address, uint32_t size);
Boot_StatusTypeDef BootFlashRead(uint32_t address, void *data, uint32_t size);
Boot_StatusTypeDef BootFlashWrite(uint32_t address, const void *data, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* FLASH_H_ */