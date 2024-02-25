#include "com.h"
#include <stddef.h>

// Make sure only one protocol is used at a time
#if defined(USE_CAN) && defined(USE_UART)
    #error "Only one protocol can be used at a time"
#endif

// Ensure that multiple interfaces cannot be defined
#ifdef USE_UART
    #include "uart.h"
    #define INTERFACE_INIT() UARTInit()
    #define INTERFACE_DEINIT() UARTDeInit()
    #define INTERFACE_TRANSMIT(X1, X2, X3) UARTTransmit(X1, X2)
    #define INTERFACE_RECEIVE(X1, X2, X3) UARTReceive(X1, X2, X3)
#endif

Boot_StatusTypeDef ComInit(void) {
    return INTERFACE_INIT();
}

Boot_StatusTypeDef ComDeInit(void) {
    return INTERFACE_DEINIT();
}

inline Boot_StatusTypeDef ComTransmit(uint8_t *data, uint8_t length, uint32_t timeout_ms) {
    return INTERFACE_TRANSMIT(data, length, timeout_ms);
}

inline Boot_StatusTypeDef ComReceive(uint8_t *data, uint8_t length, uint32_t timeout_ms) {
    return INTERFACE_RECEIVE(data, length, timeout_ms);
}

Boot_StatusTypeDef ComTransmitPacket(Boot_MsgIdTypeDef msg_id, uint8_t *data, uint8_t length) {
    Boot_StatusTypeDef status = BOOT_OK;

    // Transmit the message ID
    status = ComTransmit((uint8_t *)&msg_id, 1, BYTE_TIMEOUT_MS);
    if (status != BOOT_OK)
        return status;

    // Transmit the length
    status = ComTransmit(&length, 1, BYTE_TIMEOUT_MS);
    if (status != BOOT_OK)
        return status;

    // Transmit the data
    status = ComTransmit(data, length, BYTE_TIMEOUT_MS);
    if (status != BOOT_OK)
        return status;

    return BOOT_OK;
}

Boot_StatusTypeDef ComReceivePacket(Boot_MsgIdTypeDef *msg_id, uint8_t *data, uint8_t *length, uint32_t timeout_ms) {
    Boot_StatusTypeDef status = BOOT_OK;

    // Given timeout only applies to the message ID bytes
    // All other bytes are expected to be received within BYTE_TIMEOUT_MS
    // Timeout on other bytes leads to a format error, not timeout error
    status = ComReceive((uint8_t*)msg_id, 1, timeout_ms);
    if (status != BOOT_OK)
        return BOOT_TIMEOUT;

    // Receive the length
    status = ComReceive((uint8_t*)length, 1, BYTE_TIMEOUT_MS);
    if (status != BOOT_OK)
        return BOOT_FORMAT_ERROR;

    // Receive the data
    status = ComReceive(data, *length, BYTE_TIMEOUT_MS);
    if (status != BOOT_OK)
        return BOOT_FORMAT_ERROR;

    return BOOT_OK;
}

inline Boot_StatusTypeDef ComAck() {
    return ComTransmitPacket(MSG_ID_ACK, NULL, 0);
}

inline Boot_StatusTypeDef ComNack() {
    return ComTransmitPacket(MSG_ID_NACK, NULL, 0);
}
