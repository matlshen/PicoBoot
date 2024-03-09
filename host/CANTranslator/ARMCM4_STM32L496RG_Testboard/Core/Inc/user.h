#include "can.h"
#include "uart.h"
#include "com.h"
#include "crc32.h"

static void ErrorHandler() {
    while (1) {
    }
}

#define BYTE_TIMEOUT_MS 50

Boot_StatusTypeDef CANTransmitPacket(Boot_MsgIdTypeDef msg_id, const uint8_t *data, uint16_t length) {
    bool is_data_packet = (msg_id == MSG_ID_DATA_TTH || msg_id == MSG_ID_DATA_HTT);
    
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
}

Boot_StatusTypeDef CANReceivePacket(Boot_MsgIdTypeDef *msg_id, uint8_t *data, uint16_t *length, uint32_t timeout_ms) {
    // Receive the message ID
    Boot_StatusTypeDef status = CANReceive(msg_id, data, length, BYTE_TIMEOUT_MS);
    if (status != BOOT_OK)
        return status;

    bool is_data_packet = (*msg_id == MSG_ID_DATA_TTH || *msg_id == MSG_ID_DATA_HTT);
    // If not a data packet, receive a single CAN frame
    if (!is_data_packet)
        return CANReceive(msg_id, data, length, BYTE_TIMEOUT_MS);
    // If it is a data packet, first receive the length, then receive 8-byte CAN frames
    else {
        // Receive the length
        uint8_t rx_length;
        status = CANReceive(msg_id, &rx_length, length, BYTE_TIMEOUT_MS);
        if (status != BOOT_OK)
            return status;
        *length = rx_length;

        // Receive the data in 8-byte frames
        uint16_t bytes_remaining = *length;
        uint8_t *rx_data = data;
        while (bytes_remaining > 0) {
            uint8_t rx_length = bytes_remaining > 8 ? 8 : bytes_remaining;
            status = CANReceive(msg_id, rx_data, &rx_length, BYTE_TIMEOUT_MS);
            if (status != BOOT_OK)
                return status;
            rx_data += rx_length;
            bytes_remaining -= rx_length;
        }

        // Receive and check the CRC
        uint32_t rx_crc;
        status = CANReceive(msg_id, (uint8_t*)&rx_crc, length, BYTE_TIMEOUT_MS);
        if (status != BOOT_OK)
            return status;
        if (crc32(data, *length, INITIAL_CRC) != rx_crc)
            return BOOT_ERROR;
    }

    return BOOT_OK;
}

void UserSetup() {
    ComInit();

    Boot_MsgIdTypeDef rx_id;
    uint8_t rx_data[256];
    uint16_t rx_length;

    uint8_t test_data[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0x0A, 0x0B, 0x0C, 0x0D};
    uint8_t test_data_11[11];
    for (int i = 0; i < 11; i++) {
        test_data_11[i] = i%4;
    }
    uint8_t test_data_long[256];
    for (int i = 0; i < 256; i++) {
        test_data_long[i] = i%5;
    }

    CANInit();

    while (1) {
        // Receive packet from host over UART
        if (ComReceivePacket(&rx_id, rx_data, &rx_length, 5000) == BOOT_OK) {
            // If this was a data packet, keep receiving until the entire packet is received

            // Receive packet from target over CAN
            if (CANReceivePacket(&rx_id, rx_data, &rx_length, 5000) == BOOT_OK) {
                // Transmit packet to host over UART
                ComTransmitPacket(rx_id, rx_data, rx_length);
            }
        }
    }
}