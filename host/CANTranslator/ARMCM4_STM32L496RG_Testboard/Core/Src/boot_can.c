#include "can.h"
#include "stm32l4xx_hal.h"

extern CAN_HandleTypeDef hcan1;

Boot_StatusTypeDef CANInit(void) {
    // Initialize CAN peripheral at 500 kbps
    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 16;
    hcan1.Init.Mode = CAN_MODE_LOOPBACK;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_8TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = DISABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = DISABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&hcan1) != HAL_OK)
        return BOOT_ERROR;

    // Only accept messages with ID in 0x700-0x7FF range
    CAN_FilterTypeDef sFilterConfig = {
        .FilterBank = 0,
        .FilterMode = CAN_FILTERMODE_IDMASK,
        .FilterScale = CAN_FILTERSCALE_32BIT,
        // The ID and mask need to be shifted left by 5 bits for standard IDs
        .FilterIdHigh = (0x700 << 5) & 0xFFFF, // Shift left by 5, keep only high 16 bits
        .FilterMaskIdHigh = (0x7FF << 5) & 0xFFFF, // A mask of 0x7FF will not work as intended for a range
        .FilterFIFOAssignment = CAN_RX_FIFO0,
        .FilterActivation = ENABLE
    };  
    HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);

    // Start the CAN peripheral
    HAL_CAN_Start(&hcan1);

    return BOOT_OK;
}

Boot_StatusTypeDef CANDeInit(void) {
    HAL_CAN_Stop(&hcan1);
    HAL_CAN_DeInit(&hcan1);

    return BOOT_OK;
}

Boot_StatusTypeDef CANTransmit(uint16_t id, const uint8_t *data, uint8_t length, uint32_t timeout_ms) {
    CAN_TxHeaderTypeDef TxHeader;
    TxHeader.DLC = length;
    TxHeader.StdId = id;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;

    uint32_t TxMailbox;

    // Spinlock until all tx mailboxes are empty
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) != 3);

    HAL_CAN_AddTxMessage(&hcan1, &TxHeader, data, &TxMailbox);

    return BOOT_OK;
}

Boot_StatusTypeDef CANReceive(uint16_t *id, uint8_t *data, uint8_t *length, uint32_t timeout_ms) {
    CAN_RxHeaderTypeDef RxHeader;

    // Spinlock until a message is received
    while (!HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0));

    HAL_StatusTypeDef status = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, data);

    if (status == HAL_OK) {
        *id = RxHeader.StdId;
        *length = RxHeader.DLC;
        return BOOT_OK;
    } else {
        return BOOT_ERROR;
    }
}