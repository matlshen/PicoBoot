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

    // Receive config
    Boot_DataPacketTypeDef data_packet;
    if (ComReceivePacket(&data_packet.msg_id, data_packet.data, &data_packet.length, BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
        return BOOT_ERROR;

    // Check if received message is a config response
    if (data_packet.msg_id != MSG_ID_DATA_TTH || data_packet.length != sizeof(Boot_ConfigTypeDef))
        return BOOT_ERROR;

    // Copy received config to target_config
    memcpy(&target_config, data_packet.data, sizeof(Boot_ConfigTypeDef));

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
    if (ComTransmitPacket(MSG_ID_DATA_HTT, (const uint8_t*)&target_config, sizeof(Boot_ConfigTypeDef)) != BOOT_OK)
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
    int isize = size;
    while (isize > 0) {
        Boot_MsgIdTypeDef data_msg_id;
        uint16_t data_length;
        if (ComReceivePacket(&data_msg_id, data, &data_length, BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
            return BOOT_ERROR;

        if (data_msg_id != MSG_ID_DATA_TTH)
            return BOOT_ERROR;

        data += data_length;
        isize -= data_length;

        // If underflow, return error
        if (isize < 0)
            return BOOT_ERROR;

        // Send ACK if more data is expected
        if (isize > 0 )
            ComAck();
    }

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
        uint16_t chunk_size = size > 256 ? 256 : size;
        if (ComTransmitPacket(MSG_ID_DATA_HTT, data, chunk_size) != BOOT_OK)
            return BOOT_ERROR;

        // Wait for ACK
        if (WaitForAck(BL_COMMAND_TIMEOUT_MS) != BOOT_OK)
            return BOOT_ERROR;

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
    uint16_t length = 0xFF;

    // Wait for ack
    if (ComReceivePacket(&msg_id, NULL, &length, timeout_ms) != BOOT_OK)
        return BOOT_ERROR;

    // Check if received message is an ACK
    if (msg_id != MSG_ID_ACK || length != 0)
        return BOOT_ERROR;

    return BOOT_OK;
}