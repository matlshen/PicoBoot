#include "uart.h"

#include "timer.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_usart.h"

#define UARTx USART2

#define UART_DEVICE LL_APB1_GRP1_PERIPH_USART2
#define GPIO_DEVICE LL_AHB1_GRP1_PERIPH_GPIOA

#define USART_TX_Pin        LL_GPIO_PIN_2
#define USART_TX_GPIO_Port  GPIOA
#define USART_RX_Pin        LL_GPIO_PIN_3
#define USART_RX_GPIO_Port  GPIOA


Boot_StatusTypeDef UARTInit(void) {
    LL_USART_InitTypeDef USART_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable peripheral clocks
    LL_APB1_GRP1_EnableClock(UART_DEVICE);
    LL_AHB1_GRP1_EnableClock(GPIO_DEVICE);

    // Configure TX pin GPIO
    GPIO_InitStruct.Pin = USART_TX_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
    LL_GPIO_Init(USART_TX_GPIO_Port, &GPIO_InitStruct);

    // Configure RX pin GPIO
    GPIO_InitStruct.Pin = USART_RX_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
    LL_GPIO_Init(USART_RX_GPIO_Port, &GPIO_InitStruct);

    // Configure UART settings
    USART_InitStruct.BaudRate = 115200;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(UARTx, &USART_InitStruct);
    LL_USART_DisableIT_CTS(UARTx);
    LL_USART_ConfigAsyncMode(UARTx);
    LL_USART_Enable(UARTx);

    // Disable receive to prevent overrun
    LL_USART_DisableDirectionRx(UARTx);

    return BOOT_OK;
}

Boot_StatusTypeDef UARTReceive(uint8_t *data, uint8_t length, uint32_t timeout_ms) {
    // Enable receive
    LL_USART_EnableDirectionRx(UARTx);

    TimerSetTimeout(timeout_ms);

    while (length--) {
        while (!LL_USART_IsActiveFlag_RXNE(UARTx))
            if (TimerUpdate() == BOOT_TIMEOUT)
                return BOOT_TIMEOUT;

        *data++ = LL_USART_ReceiveData8(UARTx);

        // For each subsequent byte, set timeout to byte timeout
        TimerSetTimeout(UART_BYTE_TIMEOUT_MS);
    }

    // Disable receive to prevent overrun
    LL_USART_DisableDirectionRx(UARTx);

    return BOOT_OK;
}

Boot_StatusTypeDef UARTTransmit(uint8_t *data, uint8_t length, uint32_t timeout_ms) {
    TimerSetTimeout(timeout_ms);

    while (length--) {
        while (!LL_USART_IsActiveFlag_TXE(UARTx))
            if (TimerUpdate() == BOOT_TIMEOUT)
                return BOOT_TIMEOUT;

        LL_USART_TransmitData8(UARTx, *data++);
    }

    return BOOT_OK;
}