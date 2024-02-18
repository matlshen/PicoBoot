#ifndef BOOT_H_
#define BOOT_H_

#include <stdint.h>
#include <stdbool.h>
#include "boot_config.h"
#include "boot_types.h"
#include "com.h"
#include "crc32.h"

#ifdef __cplusplus
extern "C" {
#endif

void BootStateMachine(void);
static void Init(void);
static void WaitForConnection(void);
static void WaitForCommand(void);

#ifdef __cplusplus
}
#endif

#endif // BOOT_H_