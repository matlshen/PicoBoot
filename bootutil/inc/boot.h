#ifndef BOOT_H_
#define BOOT_H_

#include <stdint.h>
#include <stdbool.h>
#include <memory.h>
#include "boot_config.h"
#include "boot_types.h"
#include "ll_util.h"
#include "flash.h"
#include "flash_util.h"
#include "com.h"
#include "crc32.h"
#include "sha256.h"

#ifdef __cplusplus
extern "C" {
#endif

extern Boot_ConfigTypeDef* p_cfg;

void BootStateMachine(void);

bool VerifySlot(uint8_t slot);
uint32_t GetConfigCrc(Boot_ConfigTypeDef* p_boot_config);


#ifdef __cplusplus
}
#endif

#endif // BOOT_H_