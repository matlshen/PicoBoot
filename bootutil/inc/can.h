#ifndef CAN_H_
#define CAN_H_

#include <stdint.h>
#include <stddef.h>
#include "boot_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CAN_BYTE_TIMEOUT_MS 100
#define CAN_ID(msg_id) ((uint16_t)msg_id + 0x700)
#define CAN_MSG_ID(can_id) (Boot_MsgIdTypeDef)(can_id - 0x700)

Boot_StatusTypeDef CANInit(void);
Boot_StatusTypeDef CANDeInit(void);
Boot_StatusTypeDef CANTransmit(uint16_t id, const uint8_t *data, uint8_t length, uint32_t timeout_ms);
Boot_StatusTypeDef CANReceive(uint16_t *id, uint8_t *data, uint8_t *length, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* CAN_H_ */