#include "uart.h"
#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_bus.h"

// TODO: Make this configurable somehow
#define UARTx USART2            // UART peripheral
#define UART_RX_PIN (1U << 3)   // Rx on PA3
#define UART_TX_PIN (1U << 2)   // Tx on PA2

Boot_StatusTypeDef UARTInit(void) {
    LL_USART_InitTypeDef USART_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable peripheral clocks
    // TODO: Make this configurable somehow
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);

    // Configure GPIO pin alternate function
    GPIO_InitStruct.Pin = UART_TX_PIN | UART_RX_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configure UART parameters
    USART_InitStruct.BaudRate = 115200;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(UARTx, &USART_InitStruct);
    LL_USART_ConfigAsyncMode(UARTx);
    LL_USART_Enable(UARTx);

    // Disable receive to prevent overrun
    LL_USART_DisableDirectionRx(UARTx);

    return BOOT_OK;
}

Boot_StatusTypeDef UARTDeInit(void) {
    // Disable UART
    LL_USART_Disable(UARTx);

    // Disable peripheral clocks
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART2);
    LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOA);

    return BOOT_OK;
}

Boot_StatusTypeDef UARTReceive(uint8_t *data, uint32_t length, uint32_t timeout_ms) {
    // Enable receive
    LL_USART_EnableDirectionRx(UARTx);

    // first_byte is used to determine if we should wait for the full timeout period
    bool first_byte = true;

    while (length--) {
        uint32_t start_time = HAL_GetTick();

        // Wait for byte to be received
        // For first byte, wait for timeout period
        // For subsequent bytes, wait for shorter byte timeout period
        while (!LL_USART_IsActiveFlag_RXNE(UARTx))
            if ((HAL_GetTick() - start_time) > (first_byte ? timeout_ms : UART_BYTE_TIMEOUT_MS))
                return first_byte ? BOOT_TIMEOUT : BOOT_FORMAT_ERROR;

        first_byte = false;

        // Read byte
        *data++ = LL_USART_ReceiveData8(UARTx);
    }

    // Disable receive to prevent overrun
    LL_USART_DisableDirectionRx(UARTx);

    return BOOT_OK;
}

Boot_StatusTypeDef UARTTransmit(const uint8_t *data, uint32_t length, uint32_t timeout_ms) {
    // first_byte is used to determine if we should wait for the full timeout period
    bool first_byte = true;

    while (length--) {
        uint32_t start_time = HAL_GetTick();

        // Wait for byte to be sent
        // For first byte, wait for timeout period
        // For subsequent bytes, wait for shorter byte timeout period
        while (!LL_USART_IsActiveFlag_TXE(UARTx))
            if ((HAL_GetTick() - start_time) > (first_byte ? timeout_ms : UART_BYTE_TIMEOUT_MS))
                return first_byte ? BOOT_TIMEOUT : BOOT_FORMAT_ERROR;

        first_byte = false;

        // Transmit data
        LL_USART_TransmitData8(UARTx, *data++);
    }

    return BOOT_OK;
}