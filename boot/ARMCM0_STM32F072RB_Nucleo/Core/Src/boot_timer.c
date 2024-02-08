#include "timer.h"

#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_tim.h"

uint32_t boot_time_ms = 0;
uint32_t timeout_time_ms = BL_TIMEOUT_MS;

void TimerInit(void) {
    // Enable TIM2 clock
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

    // Set prescaler to generate 1ms ticks
    LL_TIM_SetPrescaler(TIM2, __LL_TIM_CALC_PSC(SystemCoreClock, 1000));
    // Generate an update event to update prescaler value immediately
    LL_TIM_GenerateEvent_UPDATE(TIM2);

    // Set timer to count up
    LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);

    // Zero timer
    LL_TIM_SetCounter(TIM2, 0);

    // Enable TIM2 counter
    LL_TIM_EnableCounter(TIM2);
}

Boot_StatusTypeDef TimerUpdate(void) {
    // Read timer count
    boot_time_ms = TIM2->CNT;

    // Check if timeout expired
    if (boot_time_ms > timeout_time_ms) {
        return BOOT_TIMEOUT;
    }

    return BOOT_OK;
}

void TimerSetTimeout(uint32_t timeout_ms) {
    // Read timer count
    boot_time_ms = TIM2->CNT;

    timeout_time_ms = boot_time_ms + timeout_ms;

    // If overflow, set to max
    if (timeout_time_ms < boot_time_ms) {
        timeout_time_ms = __UINT32_MAX__;
    }
}

void TimerDelay(uint32_t delay_ms) {
    // Preserve original timeout
    uint32_t timeout_store = timeout_time_ms;

    // Set timeout
    TimerSetTimeout(delay_ms);

    // Wait for timeout
    while (boot_time_ms < timeout_time_ms) {
        // Read timer count
        boot_time_ms = TIM2->CNT;
    }

    // Restore timeout
    timeout_time_ms = timeout_store;
}

void TimerDeInit(void) {
    LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_TIM2);
    LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_TIM2);
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM2);

    // Reset SysTick as well
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
}