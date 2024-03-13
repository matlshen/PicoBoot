#include "user.h"

#include "cmsis_os.h"

#define BYTE_TIMEOUT_MS 100

/* UART to CAN thread definitions */
osThreadId_t uart2can_thread_id;
uint32_t uart2can_thread_stack[128];
StaticTask_t uart2can_thread_tcb;
const osThreadAttr_t uart2can_thread_attr = {
    .name = "uart2can",
    .stack_mem = &uart2can_thread_stack,
    .stack_size = sizeof(uart2can_thread_stack),
    .priority = osPriorityNormal,
    .cb_mem = &uart2can_thread_tcb,
    .cb_size = sizeof(uart2can_thread_tcb)
};

/* CAN to UART thread definitions */
osThreadId_t can2uart_thread_id;
uint32_t can2uart_thread_stack[128];
StaticTask_t can2uart_thread_tcb;
const osThreadAttr_t can2uart_thread_attr = {
    .name = "can2uart",
    .stack_mem = &can2uart_thread_stack,
    .stack_size = sizeof(can2uart_thread_stack),
    .priority = osPriorityNormal,
    .cb_mem = &can2uart_thread_tcb,
    .cb_size = sizeof(can2uart_thread_tcb)
};

Boot_StatusTypeDef UARTTransmitPacket(Boot_MsgIdTypeDef msg_id, const uint8_t *data, uint16_t length) {
    bool is_data_packet = (msg_id == MSG_ID_DATA_TTH || msg_id == MSG_ID_DATA_HTT);

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

}

Boot_StatusTypeDef UARTReceivePacket(Boot_MsgIdTypeDef *msg_id, uint8_t *data, uint16_t *length, uint32_t timeout_ms) {
    bool isDataPacket = false;

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

    return BOOT_OK;
}

void UserSetup(void) {
    // Initialize the CAN interface (controlled by COM driver)
    ComInit();

    // Initialize virtual COM port
    UARTInit();

    // Create the UART to CAN thread
    uart2can_thread_id = osThreadNew(UART2CANThread, NULL, &uart2can_thread_attr);

    // Create the CAN to UART thread
    can2uart_thread_id = osThreadNew(CAN2UARTThread, NULL, &can2uart_thread_attr);

    // Start the scheduler
    osKernelStart();
}

void UART2CANThread(void *argument) {
    Boot_MsgIdTypeDef msg_id;
    uint8_t data[256] = {0};
    uint16_t length = 0;

    while (1) {
        if (UARTReceivePacket(&msg_id, data, &length, 10000) == BOOT_OK) {
            ComTransmitPacket(msg_id, data, length);
        }
    }
}

void CAN2UARTThread(void *argument) {
    Boot_MsgIdTypeDef msg_id;
    uint8_t data[256] = {0};
    uint16_t length = 0;

    while (1) {
        if (ComReceivePacket(&msg_id, data, &length, 10000) == BOOT_OK) {
            UARTTransmitPacket(msg_id, data, length);
        }
    }
}