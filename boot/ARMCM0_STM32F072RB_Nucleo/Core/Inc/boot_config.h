#ifndef BOOT_CONFIG_H_
#define BOOT_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

// #define HOST
#define TARGET

#define APP_START_ADDRESS           0x8004000U

#define BL_FLASH_START_ADDRESS      0x8000000U
#define BL_FLASH_PAGE_SIZE          0x400
#define BL_FLASH_SECTOR_SIZE        0x1000
#define BL_FLASH_SIZE               0x00020000U
#define BL_FLASH_END_ADDRESS        BL_FLASH_START_ADDRESS + BL_FLASH_SIZE

#define BL_SRAM_START_ADDRESS       0x20000000U
#define BL_SRAM_SIZE                0x00060000U

// Timeout from startup to jump to application
#define BL_TIMEOUT_MS           100000U
// Timeout for commands
#define COMMAND_TIMEOUT_MS      100000U

#define SYSTEM_CLOCK_MHZ        16U

// #define USE_CAN
//     #define CAN_RX_PIN      PB8
//     #define CAN_TX_PIN      PB9
//     #define CAN_BAUDRATE    500000
#define USE_UART
    #define UART_RX_PIN_PA3
    #define UART_TX_PIN_PA2
    #define UART_BAUDRATE   115200
// #define USE_PC_SERIAL
//     #define PC_SERIAL_BAUDRATE 115200


#ifdef __cplusplus
}
#endif

#endif // BOOT_CONFIG_H_