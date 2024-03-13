#ifndef USER_H_
#define USER_H_



#include "can.h"
#include "uart.h"
#include "com.h"
#include "crc32.h"

Boot_StatusTypeDef UARTTransmitPacket(Boot_MsgIdTypeDef msg_id, const uint8_t *data, uint16_t length);
Boot_StatusTypeDef UARTReceivePacket(Boot_MsgIdTypeDef *msg_id, uint8_t *data, uint16_t *length, uint32_t timeout_ms);

void UserSetup(void);

void UART2CANThread(void *argument);
void CAN2UARTThread(void *argument);

#endif // USER_H_