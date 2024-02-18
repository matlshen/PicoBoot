#ifndef FLASH_UTIL_H_
#define FLASH_UTIL_H_

#include <stdint.h>
#include <stdbool.h>
#include "boot_config.h"

#ifdef __cplusplus
extern "C" {
#endif

bool FlashUtil_IsRangeValid(uint32_t address, uint32_t size);
bool FlashUtil_IsPageAligned(uint32_t value);
uint32_t FlashUtil_GetRangeAlignment(uint32_t address, uint32_t size);


#ifdef __cplusplus
}
#endif

#endif /* FLASH_UTIL_H_ */