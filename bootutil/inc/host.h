#ifndef HOST_H_
#define HOST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "boot.h"

Boot_StatusTypeDef ConnectToTarget(void);
Boot_StatusTypeDef EraseTargetMemory(uint32_t address, uint16_t size);
Boot_StatusTypeDef ReadTargetMemory(uint32_t address, uint16_t size, uint8_t *data);
Boot_StatusTypeDef WriteTargetMemory(uint32_t address, uint16_t size, uint8_t *data);

Boot_StatusTypeDef WaitForAck(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif // HOST_H_