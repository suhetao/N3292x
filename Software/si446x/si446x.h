/*
==========================未经授权，转载需保留此信息============================
Function : MAIN.C file Operation for nRF24L01
Taobao website:http://zeyaotech.taobao.com/
Official website:http://www.ashining.com/
Written by:Lijian
TEL:028-64891123(Sales),028-64891129(Technology),e-mail: service@ashining.com
================================================================================
*/

#ifndef _SI446X_H_
#define _SI446X_H_

#include "si446x_defs.h"

/*
=================================================================================
-----------------------------External IMPORT functions---------------------------
=================================================================================
*/

#include "gpio.h"
#include "spidev.h"

extern spidev g_spidev;
extern gpio g_CS;
extern gpio g_SDN;
extern gpio g_IRQ;

#define SI_CSN_LOW( ) gpio_set(&g_CS, 0);
#define SI_CSN_HIGH( )  gpio_set(&g_CS, 1);

#define SI_SDN_LOW( )   gpio_set(&g_SDN, 0);
#define SI_SDN_HIGH( )  gpio_set(&g_SDN, 1);

/*
=================================================================================
------------------------------INTERNAL EXPORT APIs-------------------------------
=================================================================================
*/

#define  PACKET_LENGTH      0 //0-64, if = 0: variable mode, else: fixed mode



/*
=================================================================================
------------------------------INTERNAL EXPORT APIs-------------------------------
=================================================================================
*/

/*Read the PART_INFO of the device, 8 bytes needed*/
void SI446X_PART_INFO( uint8_t *buffer );

/*Read the FUNC_INFO of the device, 7 bytes needed*/
void SI446X_FUNC_INFO( uint8_t *buffer );

/*Send a command to the device*/
void SI446X_CMD( uint8_t *cmd, uint8_t cmdsize );

/*Read the INT status of the device, 9 bytes needed*/
void SI446X_INT_STATUS( uint8_t *buffer );

/*Read the PROPERTY of the device*/
void SI446X_GET_PROPERTY_X( SI446X_PROPERTY GROUP_NUM, uint8_t NUM_PROPS, uint8_t *buffer  );

/*configuration the device*/
void SI446X_CONFIG_INIT( void );

/*reset the SI446x device*/
void SI446X_RESET( void );

/*write data to TX fifo*/
void SI446X_W_TX_FIFO( uint8_t *txbuffer, uint8_t size );

/*start TX command*/
void SI446X_START_TX( uint8_t channel, uint8_t condition, uint16_t tx_len );

/*read RX fifo*/
uint8_t SI446X_READ_PACKET( uint8_t *buffer );

/*start RX state*/
void SI446X_START_RX( uint8_t channel, uint8_t condition, uint16_t rx_len,
					 uint8_t n_state1, uint8_t n_state2, uint8_t n_state3 );

/*read packet information*/
void SI446X_PKT_INFO( uint8_t *buffer, uint8_t FIELD, uint16_t length, uint16_t diff_len );

/*read fifo information*/
void SI446X_FIFO_INFO( uint8_t *buffer );

/*Power up the device*/
void SI446X_POWER_UP( uint32_t f_xtal );

/*send a packet*/
void SI446X_SEND_PACKET( uint8_t *txbuffer, uint8_t size, uint8_t channel, uint8_t condition );

/*Set the PROPERTY of the device*/
void SI446X_SET_PROPERTY_X( SI446X_PROPERTY GROUP_NUM, uint8_t NUM_PROPS, uint8_t *PAR_BUFF );

/*config the CRC, PROPERTY 0x1200*/
void SI446X_CRC_CONFIG( uint8_t PKT_CRC_CONFIG );

/*Get the PROPERTY of the device, only 1 byte*/
uint8_t SI446X_GET_PROPERTY_1( SI446X_PROPERTY GROUP_NUM );

/*Set the PROPERTY of the device, only 1 byte*/
void SI446X_SET_PROPERTY_1( SI446X_PROPERTY GROUP_NUM, uint8_t proirity );

/*config the GPIOs, IRQ, SDO*/
void SI446X_GPIO_CONFIG( uint8_t G0, uint8_t G1, uint8_t G2, uint8_t G3,
						uint8_t IRQ, uint8_t SDO, uint8_t GEN_CONFIG );

/*reset the RX FIFO of the device*/
void SI446X_RX_FIFO_RESET( void );

/*reset the TX FIFO of the device*/
void SI446X_TX_FIFO_RESET( void );

uint8_t SI446X_GET_DEVICE_STATE( void );

void SI446X_CHANGE_STATE( uint8_t NewState );

void SI446X_SET_POWER( uint8_t Power_Level );
/*
=================================================================================
----------------------------PROPERTY fast setting macros-------------------------
=================================================================================
*/
// GOLBAL(0x00)
#define GLOBAL_XO_TUNE( x )                 SI446X_SET_PROPERTY_1( 0x0000, x )
#define GLOBAL_CLK_CFG( x )                 SI446X_SET_PROPERTY_1( 0x0001, x )
#define GLOBAL_LOW_BATT_THRESH( x )         SI446X_SET_PROPERTY_1( 0x0002, x )
#define GLOBAL_CONFIG( x )                  SI446X_SET_PROPERTY_1( 0x0003, x )
#define GLOBAL_WUT_CONFIG( x )              SI446X_SET_PROPERTY_1( 0x0004, x )
#define GLOBAL_WUT_M_15_8( x )              SI446X_SET_PROPERTY_1( 0x0005, x )
#define GLOBAL_WUT_M_7_0( x )               SI446X_SET_PROPERTY_1( 0x0006, x )
#define GLOBAL_WUT_R( x )                   SI446X_SET_PROPERTY_1( 0x0007, x )
#define GLOBAL_WUT_LDC( x )                 SI446X_SET_PROPERTY_1( 0x0008, x )
#define GLOBAL_WUT_CAL( x )                 SI446X_SET_PROPERTY_1( 0x0009, x )

// INT_CTL(0x01)
#define INT_CTL_ENABLE( x )                 SI446X_SET_PROPERTY_1( 0x0100, x )
#define INT_CTL_PH_ENABLE( x )              SI446X_SET_PROPERTY_1( 0x0101, x )
#define INT_CTL_MODEM_ENABLE( x )           SI446X_SET_PROPERTY_1( 0x0102, x )
#define INT_CTL_CHIP_ENABLE( x )            SI446X_SET_PROPERTY_1( 0x0103, x )

//group 0x02, FRR_CTL
#define FRR_CTL_A_MODE( x )                 SI446X_SET_PROPERTY_1( 0x0200, x )
#define FRR_CTL_B_MODE( x )                 SI446X_SET_PROPERTY_1( 0x0201, x )
#define FRR_CTL_C_MODE( x )                 SI446X_SET_PROPERTY_1( 0x0202, x )
#define FRR_CTL_D_MODE( x )                 SI446X_SET_PROPERTY_1( 0x0203, x )

// PREAMBLE (0x10)









#endif //_SI446X_H_

/*
=================================================================================
------------------------------------End of FILE----------------------------------
=================================================================================
*/
