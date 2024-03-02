#ifndef BOOT_H_
#define BOOT_H_

#include <stdint.h>
#include <stdbool.h>
#include "boot_config.h"
#include "boot_types.h"
#include "ll_util.h"
#include "flash_util.h"
#include "com.h"
#include "crc32.h"

#ifdef __cplusplus
extern "C" {
#endif

void BootStateMachine(void);
static void Init(void);
static void WaitForConnection(void);
static void WaitForCommand(void);

static void ChangeSpeed(void);
static void ChangeNodeId(void);
static void GetConfig(void);
static void SetConfig(void);
static void EraseMemory(void);
static void WriteMemory(void);
static void ReadMemory(void);
static void Verify(void);
static void Go(void);

static void HandleTimeout(void);

#ifdef __cplusplus
}
#endif

#endif // BOOT_H_