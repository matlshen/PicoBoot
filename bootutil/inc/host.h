#ifndef HOST_H_
#define HOST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "boot.h"

extern Boot_ConfigTypeDef target_config;

Boot_StatusTypeDef ConnectToTarget(uint16_t node_id);
Boot_StatusTypeDef GetTargetConfig();
Boot_StatusTypeDef SetTargetConfig();
Boot_StatusTypeDef EraseTargetMemory(uint32_t address, uint16_t size);
Boot_StatusTypeDef ReadTargetMemory(uint32_t address, uint16_t size, uint8_t *data);
Boot_StatusTypeDef WriteTargetMemory(uint32_t address, uint16_t size, uint8_t *data);
Boot_StatusTypeDef SwapTarget(uint8_t src_slot, uint8_t dst_slot);
Boot_StatusTypeDef VerifyTarget(uint8_t slot);
Boot_StatusTypeDef GoTarget();
Boot_StatusTypeDef ResetTarget();

Boot_StatusTypeDef WaitForAck(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif // HOST_H_