#include "uart.h"
#include "usbd_cdc_if.h"

typedef struct {
    uint8_t buffer[256];
    uint16_t head;
    uint16_t tail;
    uint16_t size;
} Circular_BufferTypeDef;

Circular_BufferTypeDef uart_overflow_buf = {0};

static bool overflow_buffer_put(uint8_t* data, uint8_t length) {
    if (uart_overflow_buf.size + length > sizeof(uart_overflow_buf.buffer))
        return false;

    for (int i = 0; i < length; i++) {
        uart_overflow_buf.buffer[uart_overflow_buf.head] = data[i];
        uart_overflow_buf.head = (uart_overflow_buf.head + 1) % sizeof(uart_overflow_buf.buffer);
        uart_overflow_buf.size++;
    }

    return true;
}

static bool overflow_buffer_get(uint8_t* data, uint8_t length) {
    if (uart_overflow_buf.size < length)
        return false;

    for (int i = 0; i < length; i++) {
        data[i] = uart_overflow_buf.buffer[uart_overflow_buf.tail];
        uart_overflow_buf.tail = (uart_overflow_buf.tail + 1) % sizeof(uart_overflow_buf.buffer);
        uart_overflow_buf.size--;
    }

    return true;
}

// Static variables for storing info about received data
static uint8_t* rx_data_ptr;
static volatile uint32_t rx_data_remaining;

Boot_StatusTypeDef UARTInit(void) {
    // TODO: Implement this
    return BOOT_OK;
}

Boot_StatusTypeDef UARTDeInit(void) {
    // TODO: Implement this

    return BOOT_OK;
}

Boot_StatusTypeDef UARTReceive(uint8_t* const data, uint32_t length, uint32_t timeout_ms) {
    uint32_t start_time = HAL_GetTick();
    rx_data_ptr = data;
    rx_data_remaining = length;

    // If the overflow buffer has more data than needed, copy the needed amount
    if (uart_overflow_buf.size > rx_data_remaining) {
        overflow_buffer_get(rx_data_ptr, rx_data_remaining);
        rx_data_remaining -= uart_overflow_buf.size;
        rx_data_ptr += uart_overflow_buf.size;
        return BOOT_OK;
    } 
    // Else copy the entire overflow buffer
    else {
        if (uart_overflow_buf.size > 0) {
            rx_data_remaining -= uart_overflow_buf.size;
            overflow_buffer_get(rx_data_ptr, uart_overflow_buf.size);
        }

        // Wait for the rest of the data to appear
        while (rx_data_remaining != 0)
            // Empty the overflow buffer if timeout is reached
            // This is to prevent stray bytes from misaligning the packets
            if (HAL_GetTick() - start_time > timeout_ms) {
                uart_overflow_buf.size = 0;
                uart_overflow_buf.tail = uart_overflow_buf.head;
                return BOOT_TIMEOUT;
            }
    }

    return BOOT_OK;
}

Boot_StatusTypeDef UARTTransmit(const uint8_t *data, uint32_t length, uint32_t timeout_ms) {
    uint32_t start_time = HAL_GetTick();

    // Transmit the data over virtual COM port
    while (CDC_Transmit_FS((uint8_t*)data, length) != USBD_OK)
        if (HAL_GetTick() - start_time > timeout_ms)
            return BOOT_TIMEOUT;
    
    return BOOT_OK;
}

/**
 * @brief Callback for when data is received over USB CDC
 * @param buf Buffer of data to be received
 * @param len Number of data received (in bytes)
*/
void CDC_RxCallback(uint8_t *buf, uint32_t len) {
    // If no data is to be received, do nothing
    if (rx_data_remaining == 0)
        return;

    // If bytes received is less than remaining bytes, copy the entire buffer
    if (rx_data_remaining > len) {
        memcpy(rx_data_ptr, buf, len);
        rx_data_ptr += len;
        rx_data_remaining -= len;
    }
    // Else copy the remaining bytes and store the rest in the overflow buffer
    else {
        memcpy(rx_data_ptr, buf, rx_data_remaining);

        // Store the rest of the data in the overflow buffer
        overflow_buffer_put(buf + rx_data_remaining, len - rx_data_remaining);

        // Update the remaining bytes
        rx_data_remaining = 0;
    }
}