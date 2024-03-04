#include "host.h"
#include "flash_util.h"

Boot_ConfigTypeDef target_config;

Boot_StatusTypeDef ConnectToTarget(uint16_t node_id) {
    // If node_id is 0xFFFF, do not include node_id in connection request
    if (node_id == 0xFFFF) {
        // Send empty connection request
        ComTransmitPacket(MSG_ID_CONN_REQ, NULL, 0);
    } else {
        // Send connection request with node_id
        uint8_t tx_data[2] = {(uint8_t)(node_id & 0xFF), (uint8_t)(node_id >> 8)};
        ComTransmitPacket(MSG_ID_CONN_REQ, tx_data, 2);
    }

    // Wait for ACK
    if (WaitForAck(BL_TIMEOUT_MS) != BOOT_OK)
        return BOOT_ERROR;

    return BOOT_OK;
}

Boot_StatusTypeDef GetTargetConfig() {
    // Send get config request
    ComTransmitPacket(MSG_ID_GET_CONFIG, NULL, 0);

    // Wait for ACK
    if (WaitForAck(BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
        return BOOT_ERROR;

    // Receive config
    if (ComReceive((uint8_t *)&target_config, sizeof(Boot_ConfigTypeDef), BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
        return BOOT_ERROR;

    // Wait for ACK
    if (WaitForAck(BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
        return BOOT_ERROR;

    // Check CRC on target config
    if (crc32((uint8_t *)&target_config+4, sizeof(Boot_ConfigTypeDef)-4, INITIAL_CRC) != target_config.crc32)
        return BOOT_ERROR;

    return BOOT_OK;
}

Boot_StatusTypeDef SetTargetConfig() {
    // Send set config request
    ComTransmitPacket(MSG_ID_SET_CONFIG, NULL, 0);

    // Wait for ACK
    if (WaitForAck(BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
        return BOOT_ERROR;

    // Send config
    if (ComTransmit((uint8_t *)&target_config, sizeof(Boot_ConfigTypeDef), BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
        return BOOT_ERROR;

    // Wait for ACK
    if (WaitForAck(BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
        return BOOT_ERROR;

    return BOOT_OK;
}

Boot_StatusTypeDef EraseTargetMemory(uint32_t address, uint16_t size) {
    // Send erase request
    uint8_t tx_data[6];
    ToFlashPacket(address, size, tx_data);
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

    // Receive data in 256 byte chunks
    while (size > 0) {
        uint16_t chunk_size = (size > 256) ? 256 : size;

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

    // Send write data in 256 byte chunks
    while (size > 0) {
        uint16_t chunk_size = (size > 256) ? 256 : size;

        // Send chunk
        if (ComTransmit(data, chunk_size, BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
            return BOOT_ERROR;

        // If 256 byte chunk was written or this is the last chunk, append CRC
        if (chunk_size == 256 || size == chunk_size) {
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

Boot_StatusTypeDef VerifyTarget(uint8_t slot) {
    // Send verify request
    ComTransmitPacket(MSG_ID_VERIFY, &slot, 1);

    // Wait for ACK, allow 100ms for verify to complete
    return WaitForAck(BL_COMMAND_TIMEOUT_MS + 100);
}

Boot_StatusTypeDef GoTarget() {
    // Send go request
    ComTransmitPacket(MSG_ID_GO, NULL, 0);

    // Wait for ACK
    return WaitForAck(BL_COMMAND_TIMEOUT_MS);
}

Boot_StatusTypeDef ResetTarget(void) {
    // Send reset request
    ComTransmitPacket(MSG_ID_RESET, NULL, 0);

    // Wait for ACK
    return WaitForAck(BL_COMMAND_TIMEOUT_MS);
}

/**
 * @brief  Wait for an ACK response for the specified timeout period
 * @param  timeout_ms: Maximum time to wait for an ACK response
 * @retval BOOT_OK if ACK received, BOOT_ERROR otherwise
*/
Boot_StatusTypeDef WaitForAck(uint32_t timeout_ms) {
    Boot_MsgIdTypeDef msg_id = MSG_ID_NACK;
    uint8_t length = 0xFF;

    // Wait for ack
    if (ComReceivePacket(&msg_id, NULL, &length, timeout_ms) != BOOT_OK)
        return BOOT_ERROR;

    // Check if received message is an ACK
    if (msg_id != MSG_ID_ACK || length != 0)
        return BOOT_ERROR;

    return BOOT_OK;
}