## Message Structure
All messages sent by the host and target adhere to a common format consisting of a message ID, data length code (DLC), and data byte fields (if applicable). Note that the message ID is different than the node ID, which is unique for each endpoint in the network.
| ID | DLC | Data |
|--|--|--|

The size and layout of these fields depends on the underlying serial communication protocol.

All data is sent in little-endian format. Message IDs are restricted to the range 0x50 to 0x5F to avoid conflicting with existing application IDs. 

**CAN**
- Uses standard CAN (CAN 2.0A)
- 11-bit Std ID contains message ID
- is added to each message ID to form IDs in the range 0x750 to 0x75F
- message contains the message ID and DLC within the CAN data field. Subsequent messages contain the message ID and message payload, up to 255 bytes maximum.

**UART**
- 8 bits, even parity, 1 stop bit
- only supports one target configuration
- byte ID, 1 byte DLC, maximum 255 bytes data

| Base ID | Host | Target |
|--|--|--|
0x50 |	Connection Request |	Connection Request Response
0x51 |	Change Speed Command	
0x52 |	Change Node ID Command	| Node ID Response
0x53 |	Get Configuration |	Configuration Response
0x54 |		
0x55 |		
0x56 |	Reserved	
0x57 |	Reserved	
0x58 |	Memory Erase Command	
0x59 |	Memory Read Command |	Memory Read Response
0x5A |	Memory Write Command	
0x5B |	Verify Command	
0x5C |	Go Command	
0x5D |	Reset Command	
0x5E |		ACK
0x5F |		NACK

## Messages from Host
#### Connection Request (0x50)
DLC: 2

Upon reception of a connection request, the bootloader timeout is halted and the host request timeout begins. The inclusion of the node ID field is not mandatory. Should the node ID be omitted, the DLC should be set to 0. When a node ID is specified, only targets with a matching node ID will connect. If no node ID is specified, all targets will connect. Targets will respond with ACK upon successful reception of the connection request.

#### Change Speed Request (0x731)
DLC: 4

Sets the transmission speed based on the active serial communication protocol. Target sends ACK if speed change request is successful, NACK if unsuccessful. ACK or NACK response is sent at the original transmission speed.
