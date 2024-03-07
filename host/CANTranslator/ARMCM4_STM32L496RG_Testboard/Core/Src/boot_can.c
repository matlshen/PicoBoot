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

    // TODO: Make this filter finer
    // Only accept messages with ID in 0x740-77F range
    CAN_FilterTypeDef sFilterConfig = {
        .FilterBank = 0,
        .FilterMode = CAN_FILTERMODE_IDMASK,
        .FilterScale = CAN_FILTERSCALE_32BIT,
        // Adjust the ID to start at 0x750, still shifting left by 5 for standard ID positioning
        .FilterIdHigh = (0x750 << 5) & 0xFFFF, // ID 0x750 shifted left by 5, high 16 bits
        // Adjust the mask to only allow IDs within 0x750 to 0x760 range
        // Since this range is within a 16-value span, the mask should ignore the last 4 bits
        // Thus, we need to mask out all but the last 4 bits of the relevant section after shifting
        .FilterMaskIdHigh = (~(0x3F << 5)) & 0xFFFF, // Inverting mask for last 4 bits, shifted and applied to high 16 bits
        .FilterFIFOAssignment = CAN_RX_FIFO0,
        .FilterActivation = ENABLE
    };

HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);

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
    uint32_t start_time = HAL_GetTick();
    
    CAN_RxHeaderTypeDef RxHeader;

    // Spinlock until a message is received
    while (!HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0))
        if (HAL_GetTick() - start_time > timeout_ms)
            return BOOT_TIMEOUT;

    HAL_StatusTypeDef status = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, data);

    if (status == HAL_OK) {
        if (id != NULL)
            *id = RxHeader.StdId;
        *length = RxHeader.DLC;
        return BOOT_OK;
    } else {
        return BOOT_ERROR;
    }
}