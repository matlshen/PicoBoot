#include <stdint.h>

#define FLASH_TYPEERASE_PAGES 0
#define FLASH_TYPEPROGRAM_DOUBLEWORD 1

typedef enum 
{
  HAL_OK       = 0x00U,
  HAL_ERROR    = 0x01U,
  HAL_BUSY     = 0x02U,
  HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;

typedef struct
{
  uint32_t TypeErase;
  uint32_t PageAddress;
  uint32_t NbPages;                                                
} FLASH_EraseInitTypeDef;



int HAL_FLASH_Unlock() {
    return 0;
}

int HAL_FLASH_Lock() {
    return 0;
}

int HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *PageError) {
    *PageError = 0xFFFFFFFF;
    return 0;
}