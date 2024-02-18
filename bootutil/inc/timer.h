#ifndef TIM_H_
#define TIM_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "boot_types.h"

#define BYTE_TIMEOUT_MS 500
#define MAX_TIMEOUT_MS  __UINT32_MAX__

extern uint32_t boot_time_ms;

void TimerInit(void);
Boot_StatusTypeDef TimerUpdate(void);
void TimerSetTimeout(uint32_t timeout_ms);
void TimerDelay(uint32_t delay_ms);
void TimerDeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* TIM_H_ */