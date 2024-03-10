#ifndef LL_UTIL_H_
#define LL_UTIL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void SystemReset();
void MoveVectorTable(uint32_t app_addr);
void JumpToApp(uint32_t app_addr);

#ifdef __cplusplus
}
#endif

#endif /* LL_UTIL_H_ */