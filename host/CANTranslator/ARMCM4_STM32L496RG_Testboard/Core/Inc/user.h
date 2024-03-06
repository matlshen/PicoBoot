#include "can.h"
#include "uart.h"
#include "com.h"

Boot_MsgIdTypeDef rx_id;
static uint8_t rx_data[20];
static uint8_t rx_length;
static const uint8_t test_data[7] = "Hello!";

void UserSetup() {
    CANInit();
    ComInit();

    HAL_Delay(10);
    ComTransmitPacket(MSG_ID_ACK, test_data, 7);


    while (1) {
        if (ComReceivePacket(&rx_id, rx_data, &rx_length, 10000) == BOOT_OK) {
            ComTransmitPacket(rx_id, rx_data, rx_length);
        }
        //ComReceivePacket(&rx_id, rx_data, &rx_length, 10000);
        //ComTransmitPacket(rx_id, rx_data, rx_length);
    }
}