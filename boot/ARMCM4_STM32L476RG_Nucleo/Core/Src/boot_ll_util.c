#include "ll_util.h"
#include "com.h"

#include "stm32l4xx.h"

void SystemReset() {
    NVIC_SystemReset();
}

void MoveVectorTable(uint32_t app_addr) {
    // Set the vector table to the application's vector table
    SCB->VTOR = app_addr;
}

void JumpToApp(uint32_t app_addr) {
    // Disable interrupts
    __disable_irq();

    // Deinitialize interface
    ComDeInit();

    // Deinitialize HAL
    HAL_DeInit();

    // Get reset handler address
    typedef void (*pFnResetHandler)(void);
    pFnResetHandler ResetHandler = (pFnResetHandler)(*(volatile uint32_t *)(app_addr + 4));

    // Set the vector table to the application's vector table
    SCB->VTOR = app_addr;

    // Enable interrupts
    __enable_irq();

    // Set the MSP to the value of the application's reset vector
    __set_MSP(*(volatile uint32_t *)app_addr);

    // Jump to the application's reset vector
    ResetHandler();
}