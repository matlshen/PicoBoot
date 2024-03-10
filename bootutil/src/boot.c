#include "boot.h"
#include "com.h"

#ifndef HOST

/* Function prototypes */
static void Init(void);
static void WaitForConnection(void);
static void WaitForCommand(void);
static void CheckConnection(Boot_CmdPacketTypeDef *cmd_packet);
static void ChangeSpeed(Boot_CmdPacketTypeDef *cmd_packet);
static void GetConfig(void);
static void SetConfig(Boot_CmdPacketTypeDef *cmd_packet);
static void EraseMemory(Boot_CmdPacketTypeDef *cmd_packet);
static void WriteMemory(Boot_CmdPacketTypeDef *cmd_packet);
static void ReadMemory(Boot_CmdPacketTypeDef *cmd_packet);
static void Swap(Boot_CmdPacketTypeDef *cmd_packet);
static void Verify(Boot_CmdPacketTypeDef *cmd_packet);
static void Go(void);
static void HandleTimeout(void);

/* Booloader configuration */
Boot_ConfigTypeDef* p_cfg;

/* State machine definitions */
typedef enum {
    INIT,
    WAITING_FOR_CONNECTION,
    WAITING_FOR_COMMAND,
    VERIFY,
    HANDLE_TIMEOUT,
} Boot_StateTypeDef;
static Boot_StateTypeDef boot_state = INIT;
static bool in_bootloader_mode = false;

void BootStateMachine(void) {
    switch(boot_state) {
        case INIT:
            Init();
            boot_state = WAITING_FOR_CONNECTION;
            break;
        case WAITING_FOR_CONNECTION:
            WaitForConnection();
            break;
        case WAITING_FOR_COMMAND:
            WaitForCommand();
            break;
        case HANDLE_TIMEOUT:
            HandleTimeout();
            break;
        default:
            // Shouldn't get here
            boot_state = INIT;
            break;
    }
}

// Ignore this if running emulator
#ifdef TARGET
static void Init(void) {
    // Initialize vector table offset register
    MoveVectorTable(BL_LOAD_ADDRESS);

    // Initialize communication interface
    ComInit();

    // Calculate config address as nearest page under BL_LOAD_ADDRESS
    uint32_t config_addr = FlashUtil_RoundDownToPage(BL_LOAD_ADDRESS - sizeof(Boot_ConfigTypeDef));
    
    // Check validity of boot_config
    p_cfg = (Boot_ConfigTypeDef *)config_addr;
    // If CRC32 of boot_config is not valid, store default boot_config in Flash
    if (GetConfigCrc(p_cfg) != p_cfg->crc32) {
        Boot_ConfigTypeDef default_cfg = {0};

        // Populate version
        Version_TypeDef default_version = {BL_VERSION_MAJOR, BL_VERSION_MINOR};
        default_cfg.version = default_version;

        // Populate slot information
        Slot_ConfigTypeDef default_slot0 = DEFAULT_SLOT_0_CONFIG;
        default_cfg.slot_list[0] = default_slot0;
        Slot_ConfigTypeDef default_slot1 = DEFAULT_SLOT_1_CONFIG;
        default_cfg.slot_list[1] = default_slot1;

        // Populate node ID
        default_cfg.node_id = TARGET;

        // Calculate CRC32
        default_cfg.crc32 = GetConfigCrc(&default_cfg);                                 
        BootFlashErase(config_addr, FlashUtil_RoundUpToPage(sizeof(Boot_ConfigTypeDef)));
        BootFlashWrite(config_addr, &default_cfg, sizeof(Boot_ConfigTypeDef));
    }

    // Check if reset handler at start of flash points to bootloader
    if (memcmp((const void*)BL_FLASH_START_ADDRESS, (const void*)BL_LOAD_ADDRESS, 8) != 0) {
        // If not, write bootloader reset handler to start of flash so that bootloader starts after reset
        uint8_t page_buffer[BL_FLASH_PAGE_SIZE];
        BootFlashRead(BL_FLASH_START_ADDRESS, page_buffer, BL_FLASH_PAGE_SIZE);
        BootFlashRead(BL_LOAD_ADDRESS, page_buffer, 8);
        BootFlashErase(BL_FLASH_START_ADDRESS, BL_FLASH_PAGE_SIZE);
        BootFlashWrite(BL_FLASH_START_ADDRESS, (const void*)page_buffer, BL_FLASH_PAGE_SIZE);
    }
}
#else
// If using emulator, don't load configuration
static void Init(void) {
    // Initialize communication interface
    ComInit();
}
#endif

// Wait for connection from target. Stay quiet if no connection
static void WaitForConnection(void) {
    Boot_CmdPacketTypeDef cmd_packet;

    // Wait for connection
    if (ComReceivePacket(&cmd_packet.msg_id, cmd_packet.data, &cmd_packet.length, BL_TIMEOUT_MS) != BOOT_OK) {
        return;
    }

    // If other data received, silently ignore
    if (cmd_packet.msg_id != MSG_ID_CONN_REQ)
        return;

    // If empty packet, connect
    if (cmd_packet.length == 0) {
        ComAck();
        boot_state = WAITING_FOR_COMMAND;
    }

    // If node ID is specified, check if it matches
    else if (cmd_packet.length == 2) {
        if (ToU16(&cmd_packet.data[0]) == p_cfg->node_id) {
            ComAck();
            boot_state = WAITING_FOR_COMMAND;
        }
    }
}

static void WaitForCommand(void) {
    // Wait for command
    Boot_CmdPacketTypeDef cmd_packet;
    Boot_StatusTypeDef status = ComReceivePacket(&cmd_packet.msg_id, cmd_packet.data, &cmd_packet.length, BL_TIMEOUT_MS);

    if (status == BOOT_OK) {
        // If command, handle command and go back to waiting for command
        switch (cmd_packet.msg_id) {
            case MSG_ID_CONN_REQ:
                CheckConnection(&cmd_packet);
                break;
            case MSG_ID_CHANGE_SPEED:
                // TODO: Implement this
                ChangeSpeed(&cmd_packet);
                break;
            case MSG_ID_GET_CONFIG:
                GetConfig();
                break;
            case MSG_ID_SET_CONFIG:
                SetConfig(&cmd_packet);
                break;
            case MSG_ID_MEM_ERASE:
                EraseMemory(&cmd_packet);
                break;
            case MSG_ID_MEM_WRITE:
                WriteMemory(&cmd_packet);
                break;
            case MSG_ID_MEM_READ:
                ReadMemory(&cmd_packet);
                break;
            case MSG_ID_SWAP:
                Swap(&cmd_packet);
                break;
            case MSG_ID_VERIFY:
                Verify(&cmd_packet);
                break;
            case MSG_ID_GO:
                Go();
                break;
            case MSG_ID_RESET:
                // Directly call low-level function
                // Reset can't return or produce error
                ComAck();
                ComAck(); // Ack twice to force transmit buffer flush, TODO: Make this better
                SystemReset();
                break;
            default:
                // Invalid command
                ComNack();
                break;
        }
    }

    // Timeout
    else if (status == BOOT_TIMEOUT)
        boot_state = HANDLE_TIMEOUT;
    // Other problem
    else
        ComNack();
}

void CheckConnection(Boot_CmdPacketTypeDef *cmd_packet) {
    // If length of received packet is 0, ACK and return
    if (cmd_packet->length == 0) {
        ComAck();
        return;
    }

    // If new node ID still matches, send ACK
    else if (cmd_packet->length == 2 && ToU16(&cmd_packet->data[0]) == p_cfg->node_id) {
        ComAck();
        return;
    }

    // Otherwise, silently disconnect
    boot_state = WAITING_FOR_CONNECTION;
}

void ChangeSpeed(Boot_CmdPacketTypeDef *cmd_packet) {
    // TODO: Implement this
    Boot_MsgIdTypeDef tx_msg_id = MSG_ID_CHANGE_SPEED;
    uint8_t tx_data[] = {0x92, 0x82, 0x71, 0x69};
    uint8_t tx_length = 4;
    ComTransmitPacket(tx_msg_id, tx_data, tx_length);
    ComNack();
}

void GetConfig(void) {
    ComTransmitPacket(MSG_ID_DATA_TTH, (uint8_t *)p_cfg, sizeof(Boot_ConfigTypeDef));
}

void SetConfig(Boot_CmdPacketTypeDef *cmd_packet) {
    // If length of received packet is not 0, send nack and return
    if (cmd_packet->length != 0) {
        ComNack();
        return;
    } 
    else
        ComAck();

    // Receive new configuration
    Boot_MsgIdTypeDef data_msg_id;
    Boot_ConfigTypeDef new_config;
    uint16_t data_length;
    if (ComReceivePacket(&data_msg_id, (uint8_t*)&new_config, &data_length, BL_COMMAND_TIMEOUT_MS) != BOOT_OK) {
        ComNack();
        return;
    }

    // Ensure data packet length matches size of Boot_ConfigTypeDef
    if (data_length != sizeof(Boot_ConfigTypeDef)) {
        ComNack();
        return;
    }

    // Verify CRC of new configuration
    uint32_t calculated_crc = crc32((uint8_t *)&new_config + 4, sizeof(Boot_ConfigTypeDef) - 4, INITIAL_CRC);
    if (calculated_crc != new_config.crc32) {
        ComNack();
        return;
    }

    // Write new configuration to flash
    if (BootFlashErase((uint32_t)p_cfg, FlashUtil_RoundUpToPage(sizeof(Boot_ConfigTypeDef))) != BOOT_OK) {
        ComNack();
        return;
    }
    if (BootFlashWrite((uint32_t)p_cfg, (uint8_t *)&new_config, sizeof(Boot_ConfigTypeDef)) != BOOT_OK) {
        ComNack();
        return;
    }

    // If set operation was successful, send ack
    ComAck();
}

void EraseMemory(Boot_CmdPacketTypeDef *cmd_packet) {
    // If length of received packet is not 6, send nack and return
    if (cmd_packet->length != 6) {
        ComNack();
        return;
    }

    // If erase operation was not successful, send nack
    if (FlashErase(ToU32(&cmd_packet->data[0]), ToU16(&cmd_packet->data[4])) != BOOT_OK) {
        ComNack();
        return;
    }
    
    // If erase operation was successful, send ack
    ComAck();
}

void WriteMemory(Boot_CmdPacketTypeDef *cmd_packet) {
    // If length of received packet is not 6, send nack and return
    if (cmd_packet->length != 6) {
        ComNack();
        return;
    }

    uint32_t start_address = ToU32(&cmd_packet->data[0]);
    int32_t bytes_remaining = ToU16(&cmd_packet->data[4]);
    uint32_t end_address = ToU32(&cmd_packet->data[0]) + bytes_remaining;

    // If write range is not valid, send nack and return
    // Otherwise, send ack to signal ready to receive write data
    if (!FlashWriteRangeValid(start_address, (uint32_t)bytes_remaining)) {
        ComNack();
        return;
    } else {
        ComAck();
    }

    while (bytes_remaining > 0) {
        // Receive data packet
        Boot_DataPacketTypeDef data_packet;
        if (ComReceivePacket(&data_packet.msg_id, data_packet.data, &data_packet.length, BL_COMMAND_TIMEOUT_MS) != BOOT_OK) {
            ComNack();
            return;
        }

        // Check for too many bytes received
        bytes_remaining -= data_packet.length;
        if (bytes_remaining < 0) {
            ComNack();
            return;
        }

        // Write data to flash, ack if successful, nack if not
        if (FlashWrite(end_address - bytes_remaining - data_packet.length, data_packet.data, data_packet.length) != BOOT_OK) {
            ComNack();
            return;
        } else
            ComAck();
    }
}

void ReadMemory(Boot_CmdPacketTypeDef *cmd_packet) {
    // If length of message is not 6, send nack
    if (cmd_packet->length != 6) {
        ComNack();
        return;
    }

    uint32_t start_address = ToU32(&cmd_packet->data[0]);
    uint16_t bytes_remaining = ToU16(&cmd_packet->data[4]);
    uint32_t end_address = ToU32(&cmd_packet->data[0]) + bytes_remaining;

    // If read range is not valid, send nack and return
    // Otherwise, start sending read data
    if (!FlashReadRangeValid(start_address, bytes_remaining)) {
        ComNack();
        return;
    }

    // Read data from flash into buffer in 256 byte chunks
    while (bytes_remaining > 0) {
        uint16_t current_read_size = (bytes_remaining > 256) ? 256 : bytes_remaining;

        Boot_DataPacketTypeDef data_packet;

        if (FlashRead(end_address - bytes_remaining, data_packet.data, current_read_size) != BOOT_OK) {
            ComNack();
            return;
        }

        // Update bytes remaining
        bytes_remaining -= current_read_size;

        // Transmit the current chunk
        if (ComTransmitPacket(MSG_ID_DATA_TTH, data_packet.data, current_read_size) != BOOT_OK) {
            ComNack();
            return;
        }

        // If more data needs to be send, wait for ACK
				if (bytes_remaining > 0) {
        	if (ComWaitForAck(BL_COMMAND_TIMEOUT_MS) != BOOT_OK) {
          	  ComNack();
            	return;
        	}
				}
    }
}

static void Swap(Boot_CmdPacketTypeDef *cmd_packet) {
    if (cmd_packet->length != 2) {
        ComNack();
        return;
    }

    uint8_t slot_src = cmd_packet->data[0];
    uint8_t slot_dst = cmd_packet->data[1];

    // Check if slot numbers are valid
    if (slot_src >= BL_NUM_SLOTS || slot_dst >= BL_NUM_SLOTS) {
        ComNack();
        return;
    }

    // Number of pages to swap is the maximum of the two image sizes rounded up to the nearest page
    uint32_t swap_size = p_cfg->slot_list[slot_src].image_size > p_cfg->slot_list[slot_dst].image_size ? 
                            p_cfg->slot_list[slot_src].image_size : p_cfg->slot_list[slot_dst].image_size;
    uint32_t pages_to_swap = swap_size % BL_FLASH_PAGE_SIZE == 0 ? swap_size / BL_FLASH_PAGE_SIZE : swap_size / BL_FLASH_PAGE_SIZE + 1;

    // Swap the two slots
    for (uint32_t i = 0; i < pages_to_swap; i++) {
        // Read page from source slot
        uint8_t page_buffer[BL_FLASH_PAGE_SIZE];
        if (FlashRead(p_cfg->slot_list[slot_src].load_address + i * BL_FLASH_PAGE_SIZE, page_buffer, BL_FLASH_PAGE_SIZE) != BOOT_OK) {
            ComNack();
            return;
        }

        // Erase page in destination slot
        if (FlashErase(p_cfg->slot_list[slot_dst].load_address + i * BL_FLASH_PAGE_SIZE, BL_FLASH_PAGE_SIZE) != BOOT_OK) {
            ComNack();
            return;
        }

        // Write page to destination slot
        if (FlashWrite(p_cfg->slot_list[slot_dst].load_address + i * BL_FLASH_PAGE_SIZE, page_buffer, BL_FLASH_PAGE_SIZE) != BOOT_OK) {
            ComNack();
            return;
        }
    }

    // Swap successful, send ack
    ComAck();
}

static void Verify(Boot_CmdPacketTypeDef *cmd_packet) {
    if (cmd_packet->length != 1) {
        ComNack();
        return;
    }
    
    if (cmd_packet->data[0] > BL_NUM_SLOTS) {
        ComNack();
        return;
    }

    // Verify application hash in slot specified
    if (!VerifySlot(cmd_packet->data[0])) {
        ComNack();
        return;
    }
    
    ComAck();
}

void Go(void) {
    // Verify application hash in slot 0
    if (!VerifySlot(0)) {
        ComNack();
        return;
    }

    // Send ack twice to force transmit buffer flush
    ComAck();
    ComAck();

    // Go to application in slot 0
    JumpToApp(p_cfg->slot_list[0].load_address);
}

bool VerifySlot(uint8_t slot) {
    // Verify application in slot specified
    uint32_t start_addr = p_cfg->slot_list[slot].load_address;
    uint32_t size = p_cfg->slot_list[slot].image_size;

    // Compute sha256 hash of application
    uint8_t computed_hash[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (uint8_t *)start_addr, size);
    sha256_final(&ctx, computed_hash);

    // Compare computed hash with stored hash
    if (memcmp(computed_hash, p_cfg->slot_list[slot].hash, SHA256_BLOCK_SIZE) != 0)
        return false;
    else
        return true;
}

uint32_t GetConfigCrc(Boot_ConfigTypeDef* p_boot_config) {
    // Exclude first 4 bytes (CRC32) from calculation
    return crc32((uint8_t *)p_boot_config + 4, sizeof(Boot_ConfigTypeDef) - 4, INITIAL_CRC);
}

void HandleTimeout(void) {
    // If in bootloader mode, do nothing
    if (in_bootloader_mode) {
        boot_state = WAITING_FOR_COMMAND;
        return;
    }

    // If not in bootloader mode, verify application
    if (VerifySlot(0))
        Go();
    else
        // If application is not valid, enter bootloader mode
        in_bootloader_mode = true;
}

#endif // HOST