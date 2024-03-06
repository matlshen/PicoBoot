#ifndef CAN_H_
#define CAN_H_

#include <stdint.h>
#include <stddef.h>
#include "boot_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UART_BYTE_TIMEOUT_MS 100

Boot_StatusTypeDef CANInit(void);
Boot_StatusTypeDef CANDeInit(void);
Boot_StatusTypeDef CANTransmit(uint16_t id, const uint8_t *data, uint8_t length, uint32_t timeout_ms);
Boot_StatusTypeDef CANReceive(uint16_t *id, uint8_t *data, uint8_t *length, uint32_t timeout_ms);


#ifdef __cplusplus
}
#endif

#endif /* CAN_H_ */