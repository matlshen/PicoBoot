#ifndef COM_H_
#define COM_H_

#include <stdint.h>

#include "boot_types.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

Boot_StatusTypeDef ComInit(void);
Boot_StatusTypeDef ComDeInit(void);
Boot_StatusTypeDef ComTransmitPacket(Boot_MsgIdTypeDef msg_id, const uint8_t *data, uint16_t length);
Boot_StatusTypeDef ComReceivePacket(Boot_MsgIdTypeDef *msg_id, uint8_t *data, uint16_t *length, uint32_t timeout_ms);
Boot_StatusTypeDef ComAck();
Boot_StatusTypeDef ComNack();
Boot_StatusTypeDef ComWaitForAck(uint32_t timeout_ms);
uint32_t ComGetTimeoutMs();


#ifdef __cplusplus
}
#endif

#endif /* COM_H_ */