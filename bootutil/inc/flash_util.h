#ifndef FLASH_UTIL_H_
#define FLASH_UTIL_H_

#include <stdint.h>
#include <stdbool.h>
#include "boot_config.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t ToU32(const uint8_t *data);
uint16_t ToU16(const uint8_t *data);
void U32ToBytes(uint32_t value, uint8_t *data);
void U16ToBytes(uint16_t value, uint8_t *data);
void ToFlashPacket(uint32_t address, uint16_t size, uint8_t *data);
void FromFlashPacket(uint32_t *address, uint16_t *size, const uint8_t *data);

bool FlashUtil_IsRangeValid(uint32_t address, uint32_t size);
bool FlashUtil_IsPageAligned(uint32_t value);
uint32_t FlashUtil_GetPage(uint32_t address);
uint32_t FlashUtil_GetNumPages(uint32_t size);
uint32_t FlashUtil_GetRangeAlignment(uint32_t address, uint32_t size);


#ifdef __cplusplus
}
#endif

#endif /* FLASH_UTIL_H_ */