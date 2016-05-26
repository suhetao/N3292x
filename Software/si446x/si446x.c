/*
================================================================================
Function : Operation for SI446x
================================================================================
*/
#include "si446x.h"
//#include "radio_config_Si4461_30M.h"
#include "radio_config_Si446x_1k_26M.h"
static uint8_t config_table[] = RADIO_CONFIGURATION_DATA_ARRAY;
static spidev *pspidev = &g_spidev;

/*read a array of command response*/
void SI446X_READ_RESPONSE( uint8_t *buffer, uint8_t size );

/*wait the device ready to response a command*/
void SI446X_WAIT_CTS( void );

/*write data to TX fifo*/
void SI446X_W_TX_FIFO( uint8_t *txbuffer, uint8_t size );


/*!
* Sends a command to the radio chip
*
* @param byteCount     Number of bytes in the command to send to the radio device
* @param pData         Pointer to the command to send.
*/
void SI446X_CMD( uint8_t *pData, uint8_t byteCount )
{
	SI446X_WAIT_CTS( );
	SI_CSN_LOW( );
	spidev_writes(pspidev, pData, byteCount);
	SI_CSN_HIGH( );
}
/*!
* This function is used to initialize after power-up the radio chip.
* Before this function @si446x_reset should be called.
*/
void SI446X_POWER_UP( uint32_t XO_FREQ )
{
	uint8_t cmd[7];
	cmd[0] = POWER_UP;
	cmd[1] = 0x01;
	cmd[2] = 0x00;
	cmd[3] = XO_FREQ>>24;
	cmd[4] = XO_FREQ>>16;
	cmd[5] = XO_FREQ>>8;
	cmd[6] = XO_FREQ;
	SI446X_CMD( cmd, 7 );
}
/*!
* Gets a command response from the radio chip
*
* @param byteCount     Number of bytes to get from the radio chip
* @param pData         Pointer to where to put the data
*
* 
*/
void SI446X_READ_RESPONSE( uint8_t *pData, uint8_t byteCount )
{
	uint8_t cmd = READ_CMD_BUFF;

	SI446X_WAIT_CTS( );
	SI_CSN_LOW( );
	spidev_write_then_read(pspidev, &cmd, 1, pData, byteCount);
	SI_CSN_HIGH( );
}

/*!
wait the device ready to response a command
*/
void SI446X_WAIT_CTS( void )
{
	uint8_t cts;
	uint8_t cmd = READ_CMD_BUFF;
	do{
		SI_CSN_LOW( );
		spidev_write_then_read(pspidev, &cmd, 1, &cts, 1);
		SI_CSN_HIGH( );
	}while( cts != 0xFF );
}
/* Extended driver support functions */
/*!
* Sends NOP command to the radio. Can be used to maintain SPI communication.
*/
uint8_t SI446X_NOP( void )
{
	uint8_t cts;
	SI_CSN_LOW( );
	cts = spidev_write_read_duplex(pspidev, NOP );
	SI_CSN_HIGH( );
	return cts;
}

/*! This function sends the PART_INFO command to the radio and receives the answer
*  @param pData         Pointer to where to put the data
*/
void SI446X_PART_INFO( uint8_t *pData )
{
	uint8_t cmd = PART_INFO;

	SI446X_CMD( &cmd, 1 );
	SI446X_READ_RESPONSE( pData, 8 );

}

/*!
* Sends the FUNC_INFO command to the radio, then reads the resonse into @Si446xCmd union.
* @param pData         Pointer to where to put the data
*/
void SI446X_FUNC_INFO( uint8_t *pData )
{
	uint8_t cmd = FUNC_INFO;

	SI446X_CMD( &cmd, 1 );
	SI446X_READ_RESPONSE( pData, 7 );
}
/*!
* Read the INT status of the device, 9 bytes needed
* @param pData         Pointer to where to put the data
*/
void SI446X_INT_STATUS( uint8_t *pData )
{
	uint8_t cmd[4];
	cmd[0] = GET_INT_STATUS;
	cmd[1] = 0;
	cmd[2] = 0;
	cmd[3] = 0;

	SI446X_CMD( cmd, 4 );
	SI446X_READ_RESPONSE( pData, 9 );

}
/*!
* Get property values from the radio. Reads them into Si446xCmd union.
*
* @param GROUP_NUM       Property group number.
* @param NUM_PROPS   Number of properties to be read.
* @param pData         Pointer to where to put the data
*/
void SI446X_GET_PROPERTY_X( SI446X_PROPERTY GROUP_NUM, uint8_t NUM_PROPS, uint8_t *pData  )
{
	uint8_t cmd[4];

	cmd[0] = GET_PROPERTY;
	cmd[1] = GROUP_NUM>>8;
	cmd[2] = NUM_PROPS;
	cmd[3] = GROUP_NUM;

	SI446X_CMD( cmd, 4 );
	SI446X_READ_RESPONSE( pData, NUM_PROPS + 1 );
}
/*!
* Send SET_PROPERTY command to the radio.
*
* @param GROUP       Property group.
* @param NUM_PROPS   Number of property to be set. The properties must be in ascending order
*                    in their sub-property aspect. Max. 12 properties can be set in one command.
* @param pData         Pointer to where to put the data
*/
void SI446X_SET_PROPERTY_X( SI446X_PROPERTY GROUP_NUM, uint8_t NUM_PROPS, uint8_t *pData )
{
	uint8_t cmd[20], i = 0;
	if( NUM_PROPS >= 16 ){
		return;
	}
	cmd[i++] = SET_PROPERTY;
	cmd[i++] = GROUP_NUM>>8;
	cmd[i++] = NUM_PROPS;
	cmd[i++] = GROUP_NUM;
	while( NUM_PROPS-- ){
		cmd[i++] = *pData++;
	}
	SI446X_CMD( cmd, i );
}
/*
=================================================================================
Set the PROPERTY of the device, only 1 byte
* @param   GROUP_NUM, the group and number index
prioriry,  the value to be set
* @param START_PROP  Start sub-property address.
=================================================================================
*/
void SI446X_SET_PROPERTY_1( SI446X_PROPERTY GROUP_NUM, uint8_t START_PROP )
{
	uint8_t cmd[5];

	cmd[0] = SET_PROPERTY;
	cmd[1] = GROUP_NUM>>8;
	cmd[2] = 1;
	cmd[3] = GROUP_NUM;
	cmd[4] = START_PROP;
	SI446X_CMD( cmd, 5 );
}
/*

* Get the PROPERTY of the device, only 1 byte
* @param   GROUP_NUM, the group and number index
* @param   the PROPERTY value read from device

*/
uint8_t SI446X_GET_PROPERTY_1( SI446X_PROPERTY GROUP_NUM )
{
	uint8_t cmd[4];

	cmd[0] = GET_PROPERTY;
	cmd[1] = GROUP_NUM>>8;
	cmd[2] = 1;
	cmd[3] = GROUP_NUM;
	SI446X_CMD( cmd, 4 );
	SI446X_READ_RESPONSE( cmd, 2 );
	return cmd[1];
}
/*!
* This functions is used to reset the si446x radio by applying shutdown and
* releasing it.  After this function @ref si446x_boot should be called.  You
* can check if POR has completed by waiting 4 ms or by polling GPIO 0, 2, or 3.
* When these GPIOs are high, it is safe to call @ref SI446X_CONFIG_INIT.
*/
void SI446X_RESET( void )
{
	//uint16_t x = 255;
	SI_SDN_HIGH( );
	usleep(4000);
	SI_SDN_LOW( );
	SI_CSN_HIGH( );
	usleep(4000);
}
/*!
* This function is used to load all properties and commands with a list of NULL terminated commands.
* Before this function @SI446X_RESET should be called.
*/
void SI446X_CONFIG_INIT( void )
{
	uint8_t i;
	uint16_t j = 0;

	while( ( i = config_table[j] ) != 0 ){
		j += 1;
		SI446X_CMD((uint8_t*)(config_table + j), i );
		j += i;
	}
#if PACKET_LENGTH > 0           //fixed packet length
	SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_7_0, PACKET_LENGTH );
#else                           //variable packet length
	SI446X_SET_PROPERTY_1( PKT_CONFIG1, 0x00 );
	SI446X_SET_PROPERTY_1( PKT_CRC_CONFIG, 0x00 );
	SI446X_SET_PROPERTY_1( PKT_LEN_FIELD_SOURCE, 0x01 );
	SI446X_SET_PROPERTY_1( PKT_LEN, 0x2A );
	SI446X_SET_PROPERTY_1( PKT_LEN_ADJUST, 0x00 );
	SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_12_8, 0x00 );
	SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_7_0, 0x01 );
	SI446X_SET_PROPERTY_1( PKT_FIELD_1_CONFIG, 0x00 );
	SI446X_SET_PROPERTY_1( PKT_FIELD_1_CRC_CONFIG, 0x00 );
	SI446X_SET_PROPERTY_1( PKT_FIELD_2_LENGTH_12_8, 0x00 );
	SI446X_SET_PROPERTY_1( PKT_FIELD_2_LENGTH_7_0, 0x20 );
	SI446X_SET_PROPERTY_1( PKT_FIELD_2_CONFIG, 0x00 );
	SI446X_SET_PROPERTY_1( PKT_FIELD_2_CRC_CONFIG, 0x00 );
#endif //PACKET_LENGTH
	SI446X_GPIO_CONFIG( 0, 0, 33|0x40, 32|0x40, 0, 0, 0 );    //important
	//SI446X_GPIO_CONFIG( 0, 0, 0x43, 0x42, 0, 0, 0 );

}
/*!
* The function can be used to load data into TX FIFO.
*
* @param numBytes  Data length to be load.
* @param pTxData   Pointer to the data (U8*).
*/
void SI446X_W_TX_FIFO( uint8_t *pTxData, uint8_t numBytes )
{
	struct spi_ioc_transfer xfer[2] = {0};
	uint8_t cmd[1] = {WRITE_TX_FIFO};

	xfer[0].tx_buf = (unsigned long)cmd;
	xfer[0].len = 1;

	xfer[1].tx_buf = (unsigned long)pTxData;
	xfer[1].len = numBytes;

	SI_CSN_LOW( );
	//spidev_write_then_read(pspidev, &cmd, 1, pTxData, numBytes);
	if (ioctl(pspidev->fd, SPI_IOC_MESSAGE(2), xfer) < 1){
		perror("can't send spi message");
	}
	SI_CSN_HIGH( );
}
/*
send a packet
* @param pTxData, a buffer stores TX array
* @param numBytes,  how many bytes should be written
* @param channel, tx channel
* @param condition, tx condition
*/
void SI446X_SEND_PACKET( uint8_t *pTxData, uint8_t numBytes, uint8_t channel, uint8_t condition )
{
	uint8_t cmd[5];
	uint8_t tx_len = numBytes;
	//
	struct spi_ioc_transfer xfer[2] = {0};
	//
	SI446X_TX_FIFO_RESET( );

	SI_CSN_LOW( );
#if PACKET_LENGTH == 0
	tx_len++;
	cmd[0] = WRITE_TX_FIFO;
	cmd[1] = numBytes;
	xfer[0].len = 2;
#else
	cmd[0] = WRITE_TX_FIFO;
	xfer[0].len = 1;
#endif
	xfer[0].tx_buf = (unsigned long)cmd;
	xfer[1].tx_buf = (unsigned long)pTxData;
	xfer[1].len = numBytes;
	if (ioctl(pspidev->fd, SPI_IOC_MESSAGE(2), xfer) < 1){
		perror("can't send spi message");
	}
	SI_CSN_HIGH( );

	cmd[0] = START_TX;
	cmd[1] = channel;
	cmd[2] = condition;
	cmd[3] = 0;
	cmd[4] = tx_len;
	SI446X_CMD( cmd, 5 );
}
/*! Sends START_TX command to the radio.
*
* @param CHANNEL   Channel number.
* @param CONDITION Start TX condition.
* @param TX_LEN    Payload length (exclude the PH generated CRC).
*/
void SI446X_START_TX( uint8_t CHANNEL, uint8_t CONDITION, uint16_t TX_LEN )
{
	uint8_t cmd[5];

	cmd[0] = START_TX;
	cmd[1] = CHANNEL;
	cmd[2] = CONDITION;
	cmd[3] = TX_LEN>>8;
	cmd[4] = TX_LEN;
	SI446X_CMD( cmd, 5 );
}
/*
* read RX fifo
* @param pRxData  a buffer to store data read
* @param return received bytes
*/
uint8_t SI446X_READ_PACKET( uint8_t *pRxData )
{
	uint8_t length;
	uint8_t cmd = READ_RX_FIFO;

	SI446X_WAIT_CTS( );
	SI_CSN_LOW( );

#if PACKET_LENGTH == 0
	spidev_write_then_read(pspidev, &cmd, 1, &length, 1);
#else
	spidev_write_read_duplex(pspidev, cmd);
	length = PACKET_LENGTH;
#endif
	spidev_reads(pspidev, pRxData, length);
	SI_CSN_HIGH( );
	return length;
}

/*!
* Sends START_RX command to the radio.
*
* @param CHANNEL     Channel number.
* @param CONDITION   Start RX condition.
* @param RX_LEN      Payload length (exclude the PH generated CRC).
* @param NEXT_STATE1 Next state when Preamble Timeout occurs.
* @param NEXT_STATE2 Next state when a valid packet received.
* @param NEXT_STATE3 Next state when invalid packet received (e.g. CRC error).
*/
void SI446X_START_RX( uint8_t channel, uint8_t condition, uint16_t rx_len,
					 uint8_t n_state1, uint8_t n_state2, uint8_t n_state3 )
{
	uint8_t cmd[8];
	SI446X_RX_FIFO_RESET( );
	SI446X_TX_FIFO_RESET( );
	cmd[0] = START_RX;
	cmd[1] = channel;
	cmd[2] = condition;
	cmd[3] = rx_len>>8;
	cmd[4] = rx_len;
	cmd[5] = n_state1;
	cmd[6] = n_state2;
	cmd[7] = n_state3;
	SI446X_CMD( cmd, 8 );
}
/*
* reset the RX FIFO of the device
*/
void SI446X_RX_FIFO_RESET( void )
{
	uint8_t cmd[2];

	cmd[0] = FIFO_INFO;
	cmd[1] = 0x02;
	SI446X_CMD( cmd, 2 );
}
/*
* reset the TX FIFO of the device
*/
void SI446X_TX_FIFO_RESET( void )
{
	uint8_t cmd[2];

	cmd[0] = FIFO_INFO;
	cmd[1] = 0x01;
	SI446X_CMD( cmd, 2 );
}
/*
=================================================================================
SI446X_PKT_INFO( );
Function : read packet information
INTPUT   : buffer, stores the read information
FIELD, feild mask
length, the packet length
diff_len, diffrence packet length
OUTPUT   : NONE
=================================================================================
*/

/*!
* Receives information from the radio of the current packet. Optionally can be used to modify
* @param pData         Pointer to where to put the data
* @param FIELD_NUMBER_MASK Packet Field number mask value.
* @param LEN               Length value.
* @param DIFF_LEN          Difference length.
*/
void SI446X_PKT_INFO( uint8_t *pData, uint8_t FIELD_NUMBER_MASK, uint16_t LEN, uint16_t DIFF_LEN )
{
	uint8_t cmd[6];
	cmd[0] = PACKET_INFO;
	cmd[1] = FIELD_NUMBER_MASK;
	cmd[2] = LEN >> 8;
	cmd[3] = LEN;
	cmd[4] = DIFF_LEN >> 8;
	cmd[5] = DIFF_LEN;

	SI446X_CMD( cmd, 6 );
	SI446X_READ_RESPONSE( pData, 3 );
}
/*
=================================================================================
SI446X_FIFO_INFO( );
Function : read fifo information
INTPUT   : buffer, stores the read information
OUTPUT   : NONE
=================================================================================
*/

/*!
* Send the FIFO_INFO command to the radio.  
* @param pData         Pointer to where to put the data
*/
void SI446X_FIFO_INFO( uint8_t *pData )
{
	uint8_t cmd[2];
	cmd[0] = FIFO_INFO;
	cmd[1] = 0x03;

	SI446X_CMD( cmd, 2 );
	SI446X_READ_RESPONSE( pData, 3);
}

/*!
* Send GPIO pin config command to the radio and reads the answer into
* @Si446xCmd union.
*
* @param GPIO0       GPIO0 configuration.
* @param GPIO1       GPIO1 configuration.
* @param GPIO2       GPIO2 configuration.
* @param GPIO3       GPIO3 configuration.
* @param NIRQ        NIRQ configuration.
* @param SDO         SDO configuration.
* @param GEN_CONFIG  General pin configuration.
*/
void SI446X_GPIO_CONFIG( uint8_t GPIO0, uint8_t GPIO1, uint8_t GPIO2, uint8_t GPIO3,
						uint8_t IRQ, uint8_t SDO, uint8_t GEN_CONFIG )
{
	uint8_t cmd[10];
	cmd[0] = GPIO_PIN_CFG;
	cmd[1] = GPIO0;
	cmd[2] = GPIO1;
	cmd[3] = GPIO2;
	cmd[4] = GPIO3;
	cmd[5] = IRQ;
	cmd[6] = SDO;
	cmd[7] = GEN_CONFIG;
	SI446X_CMD( cmd, 8 );
	SI446X_READ_RESPONSE( cmd, 8 );
}

/*!
* Issue a change state command to the radio.
*
* @param NEXT_STATE1 Next state.
*/
void SI446X_CHANGE_STATE( uint8_t NEXT_STATE1 )
{
	uint8_t cmd[2];
	cmd[0] = CHANGE_STATE;
	cmd[1] = NEXT_STATE1;
	SI446X_CMD( cmd, 2 );
}

/*!
* Requests the current state of the device  
*/
uint8_t SI446X_GET_DEVICE_STATE( void )
{
	uint8_t cmd[3];

	cmd[0] = REQUEST_DEVICE_STATE;
	SI446X_CMD( cmd, 1 );
	SI446X_READ_RESPONSE( cmd, 3 );
	return cmd[1] & 0x0F;
}

void SI446X_SET_POWER( uint8_t Power_Level )
{
	SI446X_SET_PROPERTY_1( PA_PWR_LVL, Power_Level );
}

/*
=================================================================================
------------------------------------End of FILE----------------------------------
=================================================================================
*/
