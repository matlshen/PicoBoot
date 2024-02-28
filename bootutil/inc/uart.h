#ifndef UART_H_
#define UART_H_

#include <stdint.h>
#include <stddef.h>
#include "boot_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UART_BYTE_TIMEOUT_MS 100

Boot_StatusTypeDef UARTInit(void);
Boot_StatusTypeDef UARTDeInit(void);
Boot_StatusTypeDef UARTTransmit(const uint8_t *data, uint32_t length, uint32_t timeout_ms);
Boot_StatusTypeDef UARTReceive(uint8_t *data, uint32_t length, uint32_t timeout_ms);


#ifdef __cplusplus
}
#endif

#endif /* UART_H_ */