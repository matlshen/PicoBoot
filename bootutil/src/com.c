#include "com.h"
#include "crc32.h"
#include <stddef.h>

#define BYTE_TIMEOUT_MS 100

// Ensure that multiple interfaces cannot be defined
#ifdef USE_UART
    #include "uart.h"
#endif
#ifdef USE_CAN
    #include "can.h"
#endif

Boot_StatusTypeDef ComInit(void) {
    #ifdef USE_UART
    return UARTInit();
    #elif defined(USE_CAN)
    return CANInit();
    #endif
}

Boot_StatusTypeDef ComDeInit(void) {
    #ifdef USE_UART
    return UARTDeInit();
    #elif defined(USE_CAN)
    return CANDeInit();
    #endif
}

Boot_StatusTypeDef ComTransmitPacket(Boot_MsgIdTypeDef msg_id, const uint8_t *data, uint16_t length) {
    bool is_data_packet = (msg_id == MSG_ID_DATA_TTH || msg_id == MSG_ID_DATA_HTT);

    #ifdef USE_UART
    Boot_StatusTypeDef status = BOOT_OK;
    // Transmit the message ID
    status = UARTTransmit((uint8_t *)&msg_id, 1, BYTE_TIMEOUT_MS);
    if (status != BOOT_OK)
        return status;

    // Transmit the length
    // If this is a data packet, the length is 0x00 for 256 bytes
    uint8_t tx_length = is_data_packet && length == 256 ? 0 : length;
    status = UARTTransmit(&tx_length, 1, BYTE_TIMEOUT_MS);
    if (status != BOOT_OK)
        return status;

    // Transmit the data
    status = UARTTransmit(data, length, BYTE_TIMEOUT_MS);
    if (status != BOOT_OK)
        return status;

    // If this is a data packet, calculate and transmit the CRC
    if (is_data_packet) {
        // Calculate CRC of data
        uint32_t tx_crc = crc32(data, length, INITIAL_CRC);
        // Transmit the CRC
        status = UARTTransmit((uint8_t*)&tx_crc, sizeof(tx_crc), BYTE_TIMEOUT_MS);
        if (status != BOOT_OK)
            return status;
    }

    return BOOT_OK;

    #elif defined(USE_CAN)
    // If not a data packet, transmit a single CAN frame
    if (!is_data_packet)
        return CANTransmit(CAN_ID(msg_id), data, length, BYTE_TIMEOUT_MS);
    // If it is a data packet, first transmit the length, then transmit 8-byte CAN frames
    else {
        // Transmit the length
        uint8_t adjusted_length = length == 256 ? 0 : length;
        Boot_StatusTypeDef status = CANTransmit(CAN_ID(msg_id), &adjusted_length, 1, BYTE_TIMEOUT_MS);
        if (status != BOOT_OK)
            return status;
        
        // Transmit the data in 8-byte frames
        uint16_t bytes_remaining = length;
        const uint8_t *tx_data = data;
        while (bytes_remaining > 0) {
            uint8_t tx_length = bytes_remaining > 8 ? 8 : bytes_remaining;
            Boot_StatusTypeDef status = CANTransmit(CAN_ID(msg_id), tx_data, tx_length, BYTE_TIMEOUT_MS);
            if (status != BOOT_OK)
                return status;
            tx_data += tx_length;
            bytes_remaining -= tx_length;
        }

        // Calculate and transmit the CRC
        uint32_t tx_crc = crc32(data, length, INITIAL_CRC);
        status = CANTransmit(CAN_ID(msg_id), (uint8_t*)&tx_crc, sizeof(tx_crc), BYTE_TIMEOUT_MS);
        if (status != BOOT_OK)
            return status;
    }

    return BOOT_OK;

    #endif
}

Boot_StatusTypeDef ComReceivePacket(Boot_MsgIdTypeDef *msg_id, uint8_t *data, uint16_t *length, uint32_t timeout_ms) {
    bool isDataPacket = false;

    #ifdef USE_UART
    Boot_StatusTypeDef status = BOOT_OK;
    // Given timeout only applies to the message ID bytes
    // All other bytes are expected to be received within BYTE_TIMEOUT_MS
    // Timeout on other bytes leads to a format error, not timeout error
    status = UARTReceive((uint8_t*)msg_id, 1, timeout_ms);
    if (status != BOOT_OK)
        return BOOT_TIMEOUT;

    isDataPacket = (*msg_id == MSG_ID_DATA_TTH || *msg_id == MSG_ID_DATA_HTT);

    // Receive the length
    if (length != NULL) {
        uint8_t rx_length;
        status = UARTReceive(&rx_length, 1, BYTE_TIMEOUT_MS);
        if (status != BOOT_OK)
            return BOOT_FORMAT_ERROR;
        
        // If this is a data packet, the length is 0x00 for 256 bytes
        if (!isDataPacket)
            *length = rx_length;
        else
            *length = rx_length == 0 ? 256 : rx_length;
    }

    // Receive length bytes of data
    if (data != NULL) {
        status = UARTReceive(data, *length, BYTE_TIMEOUT_MS);
        if (status != BOOT_OK)
            return BOOT_FORMAT_ERROR;
    }
    return BOOT_OK;

    // If this is a data packet, receive and verify the CRC
    if (isDataPacket) {
        // Receive the CRC
        volatile uint32_t rx_crc;
        status = UARTReceive((uint8_t*)&rx_crc, 4, BYTE_TIMEOUT_MS);
        if (status != BOOT_OK)
            return BOOT_FORMAT_ERROR;

        // Verify the CRC
        uint32_t calc_crc = crc32(data, *length, INITIAL_CRC);
        if (rx_crc != calc_crc)
            return BOOT_FORMAT_ERROR;
    }
    
    #elif defined(USE_CAN)
    // First receive a single CAN frame
    Boot_StatusTypeDef status = BOOT_OK;
    uint16_t rx_can_id;
    status = CANReceive(&rx_can_id, data, (uint8_t*)length, timeout_ms);
    if (status != BOOT_OK)
        return status;

    // Convert the CAN ID to a message ID
    *msg_id = CAN_MSG_ID(rx_can_id);

    // If this is not a data packet, just return
    isDataPacket = (*msg_id == MSG_ID_DATA_TTH || *msg_id == MSG_ID_DATA_HTT);
    if (!isDataPacket)
        return BOOT_OK;

    // If it is a data packet, the packet just received contains the data length
    // Receive the remaining data in 8-byte CAN frames
    *length = *data == 0 ? 256 : *data;
    uint16_t remaining_length = *length;
    while (remaining_length > 0) {
        uint8_t rx_length;
        status = CANReceive(&rx_can_id, data, &rx_length, BYTE_TIMEOUT_MS);
        if (status != BOOT_OK)
            return status;
        data += rx_length;
        remaining_length -= rx_length;
    }

    // Receive and verify the CRC
    uint32_t rx_crc;
    uint8_t rx_crc_length;
    status = CANReceive(&rx_can_id, (uint8_t*)&rx_crc, &rx_crc_length, BYTE_TIMEOUT_MS);
    if (status != BOOT_OK)
        return status;
    if (*length != 4)
        return BOOT_FORMAT_ERROR;
    if (rx_crc != crc32(data, remaining_length, INITIAL_CRC))
        return BOOT_FORMAT_ERROR;

    return BOOT_OK;

    #endif
}

uint32_t ComGetTimeoutMs() {
    return BYTE_TIMEOUT_MS;
}

inline Boot_StatusTypeDef ComAck() {
    return ComTransmitPacket(MSG_ID_ACK, NULL, 0);
}

inline Boot_StatusTypeDef ComNack() {
    return ComTransmitPacket(MSG_ID_NACK, NULL, 0);
}

/**
 * @brief  Wait for an ACK response for the specified timeout period
 * @param  timeout_ms: Maximum time to wait for an ACK response
 * @retval BOOT_OK if ACK received, BOOT_ERROR otherwise
*/
Boot_StatusTypeDef ComWaitForAck(uint32_t timeout_ms) {
    Boot_CmdPacketTypeDef packet;
    uint8_t length = 0xFF;

    // Wait for ack
    if (ComReceivePacket(&packet.msg_id, NULL, &packet.length, timeout_ms) != BOOT_OK)
        return BOOT_ERROR;

    // Check if received message is an ACK
    if (packet.msg_id != MSG_ID_ACK || packet.length != 0)
        return BOOT_ERROR;

    return BOOT_OK;
}