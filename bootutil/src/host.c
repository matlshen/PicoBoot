#include "host.h"

Boot_StatusTypeDef ConnectToTarget(void) {
    // Send empty connection request
    ComTransmitPacket(MSG_ID_CONN_REQ, NULL, 0);

    // Wait for ACK
    if (WaitForAck(BL_TIMEOUT_MS) != BOOT_OK)
        return BOOT_ERROR;

    return BOOT_OK;
}

Boot_StatusTypeDef EraseTargetMemory(uint32_t address, uint16_t size) {
    // Send erase request
    uint8_t tx_data[6];
    ToFlashPacket(0, 0, tx_data);
    ComTransmitPacket(MSG_ID_MEM_ERASE, tx_data, 6);

    // Wait for ACK
    if (WaitForAck(BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
        return BOOT_ERROR;

    return BOOT_OK;
}

Boot_StatusTypeDef ReadTargetMemory(uint32_t address, uint16_t size, uint8_t *data) {
    // Send read request
    uint8_t tx_data[6];
    ToFlashPacket(address, size, tx_data);
    ComTransmitPacket(MSG_ID_MEM_READ, tx_data, 6);

    // Wait for ACK
    if (WaitForAck(BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
        return BOOT_ERROR;

    // Receive data in 255 byte chunks
    while (size > 0) {
        uint16_t chunk_size = (size > 255) ? 255 : size;

        // Receive chunk
        if (ComReceive(data, chunk_size, BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
            return BOOT_ERROR;

        data += chunk_size;
        size -= chunk_size;
    }

    // Wait for ACK
    if (WaitForAck(BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
        return BOOT_ERROR;

    return BOOT_OK;
}

Boot_StatusTypeDef WriteTargetMemory(uint32_t address, uint16_t size, uint8_t *data) {
    // Send write request
    uint8_t tx_data[6];
    ToFlashPacket(address, size, tx_data);
    ComTransmitPacket(MSG_ID_MEM_WRITE, tx_data, 6);

    // Wait for ACK
    if (WaitForAck(BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
        return BOOT_ERROR;

    // Send write data in 255 byte chunks
    while (size > 0) {
        uint16_t chunk_size = (size > 255) ? 255 : size;

        // Send chunk
        if (ComTransmit(data, chunk_size, BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
            return BOOT_ERROR;

        // If 255 byte chunk was written or this is the last chunk, append CRC
        if (chunk_size == 255 || size == chunk_size) {
            uint32_t crc = crc32(data, chunk_size, INITIAL_CRC);
            if (ComTransmit((uint8_t *)&crc, 4, BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
                return BOOT_ERROR;

            // Wait for ACK
            if (WaitForAck(BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
                return BOOT_ERROR;
        }

        data += chunk_size;
        size -= chunk_size;
    }

    return BOOT_OK;
}

/**
 * @brief  Wait for an ACK response for the specified timeout period
 * @param  timeout_ms: Maximum time to wait for an ACK response
 * @retval BOOT_OK if ACK received, BOOT_ERROR otherwise
*/
Boot_StatusTypeDef WaitForAck(uint32_t timeout_ms) {
    Boot_MsgIdTypeDef msg_id = MSG_ID_ACK;

    // Wait for ack
    if (ComReceivePacket(&msg_id, NULL, NULL, timeout_ms) != BOOT_OK)
        return BOOT_ERROR;

    return BOOT_OK;
}