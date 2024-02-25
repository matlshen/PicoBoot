#ifndef TEST_H_
#define TEST_H_

#include "uart.h"

// Data buffer
uint8_t data[256];
uint8_t error_sequence[6] = "error";

void TestFunction(void) {
    UARTInit();

    while (1) {
        // Wait for UART data to be received
        if (UARTReceive(data, 6, 50) != BOOT_OK) {
            // Transmit an error message over UART
            UARTTransmit(error_sequence, 6, 50);
        }
        else {
            // Transmit the received data back over UART
            UARTTransmit(data, 6, 50);
        }
    }
}


#endif /* TEST_H_ */