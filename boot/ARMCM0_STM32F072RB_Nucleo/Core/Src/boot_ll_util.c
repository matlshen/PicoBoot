#include "ll_util.h"

#include "stm32f0xx_hal.h"
#include "stm32f0xx_ll_crs.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_tim.h"

void LL_UtilHardwareInit() {
    HAL_Init();

    // Configure system clock to 48 MHz
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
    while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1) {}

    LL_RCC_HSI48_Enable();

    /* Wait till HSI48 is ready */
    while(LL_RCC_HSI48_IsReady() != 1) {}
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI48);

    /* Wait till System clock is ready */
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI48) {}
    LL_SetSystemCoreClock(48000000);

    LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);
}

void LL_UtilHardwareDeInit() {
    HAL_DeInit();
}

void JumpToApp() {
    // Copy vector table from application address to boot flash minus reset handler
    // This won't affect bootloader since it doesn't use interrupts

    // Only do this is MSP in flash does not already match application MSP

    // Set stack pointer to application address
    __set_MSP(*(uint32_t *)APP_START_ADDRESS);

    // Jump to application
    ((void (*)(void))(*(uint32_t *)(APP_START_ADDRESS + 4)))();
}