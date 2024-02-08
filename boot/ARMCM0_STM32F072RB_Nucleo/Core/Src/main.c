#include "main.h"

#include "timer.h"
#include "com.h"
#include "uart.h"
#include "boot.h"

void SystemClock_Config(void);

int main(void)
{
    LL_UtilHardwareInit();

    TimerInit();
    ComInit();

    uint8_t flash_data[256];
    uint8_t new_flash_data[8] = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};

    FlashRead(0x8004000, flash_data, 255);
    FlashErase(0x8004000, 0x400);
    FlashWrite(0x8004000, (uint64_t*)new_flash_data, 8);
    FlashRead(0x8004000, flash_data, 255);

    ComTransmit(flash_data, 255, 10000);

    while (1) {
        BootStateMachine();
				// status = ComReceivePacket(&rx_msg_id, rx_data, &rx_length, 5000);


				// if (status == BOOT_OK)
		    //     ComTransmitPacket(rx_msg_id, rx_data, rx_length);
        //ComTransmit((uint8_t*)"Alive\r\n", 7, 10000);
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

}


void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}