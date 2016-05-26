/*
The MIT License (MIT)
Copyright (c) 2015-? suhetao
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <termios.h> //termios
#include <unistd.h> //STDIN_FILENO getopt
#include <signal.h>
#include <string.h>

#include "buffer.h"
#include "spidev.h"
#include "gpio.h"
#include "si446x.h"

//
spidev g_spidev;
gpio g_CS;
gpio g_SDN;
gpio g_IRQ;
//
gpio g_GDO0;
gpio g_GDO1;
gpio g_GDO2;
gpio g_GDO3;
//
buffer *g_input = NULL;
//
uint8_t rf_buffer[64] = {'0','0','0','0','0',0};
//

struct termios oldt, newt;
int32_t getch(void)
{
	int ch;
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

static void ExitHandler (int sig)
{
	spidev_close(&g_spidev); 
	gpio_close(&g_CS);
	gpio_close(&g_SDN);
	gpio_close(&g_IRQ);
	gpio_close(&g_GDO0);
	gpio_close(&g_GDO1);
	gpio_close(&g_GDO2);
	gpio_close(&g_GDO3);
	buffer_destroy(g_input);

	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	printf("\n");
	exit(-9);	
}

void SI446X_TXCONFIG(void)
{
	SI446X_RESET( ); //SI446X reset
	SI446X_CONFIG_INIT( ); //SI446X init
	SI446X_SET_POWER( 0x7F );
	SI446X_CHANGE_STATE( 6 );
	while( SI446X_GET_DEVICE_STATE( ) != 6 );
	SI446X_START_RX( 0, 0, PACKET_LENGTH, 0, 0, 3);
}

void SI446X_RXCONFIG(void)
{
	SI446X_RESET( );
	SI446X_CONFIG_INIT( );
	SI446X_CHANGE_STATE( 6 );
	while( SI446X_GET_DEVICE_STATE( ) != 6 );
	SI446X_START_RX( 0, 0, PACKET_LENGTH, 0, 0, 3);
}

void SI446X_SEND_HANDLE(void)
{
	uint32_t i;
	uint8_t *data = BUFFER_GET(g_input);
	uint32_t len = BUFFER_SIZE(g_input);

	if(!len){
		return;
	}

	for(i = 0; i < len; i ++ ){
		rf_buffer[i] = data[i];
	}
	//SI446X_CHANGE_STATE( 5 );
	// while( SI446X_GET_DEVICE_STATE( ) != 5 );
#if PACKET_LENGTH == 0
	SI446X_SEND_PACKET( rf_buffer, len, 0, 0x30 );
#else
	SI446X_SEND_PACKET( rf_buffer, PACKET_LENGTH, 0, 0x30 );
#endif
	/*
	do{//
		SI446X_INT_STATUS( rf_buffer );
	}while( !( rf_buffer[3] & ( 1<<5 ) ) );
	*/
	//clear buffer
	BUFFER_RESET(g_input);
}

void SI446X_RCV_HANDLE(void)
{
	uint32_t i, length = 0;

	SI446X_INT_STATUS( rf_buffer );
	if( rf_buffer[3] & ( 1<<4 ) ){
		length = SI446X_READ_PACKET( rf_buffer );
		//
		//buffer_write(g_input, rf_buffer, length);
		for(i = 0; i < length; i++){
			printf("%c", rf_buffer[i]);
		}
		printf("\r\n");
		//
		SI446X_CHANGE_STATE( 6 );
		//while( SI446X_GET_DEVICE_STATE( ) != 6 );
		SI446X_START_RX( 0, 0, PACKET_LENGTH, 0, 0, 3);
	}

}

static void showusage()
{
	printf("si446x [options]\n");
	printf("-m tx/rx: tx or rx mode\n");
	printf("-h : Help\n");
}

int main(int argc, char **argv)
{
	char ch;
	//mode 0 for tx, 1 for rx
	int32_t opt, mode = 0;

	signal (SIGINT, ExitHandler);
	//signal (SIGQUIT, ExitHandler);
	//signal (SIGILL, ExitHandler);
	//signal (SIGABRT, ExitHandler);
	//signal (SIGFPE, ExitHandler);
	signal (SIGKILL, ExitHandler);
	//signal (SIGPIPE, ExitHandler);
	signal (SIGTERM, ExitHandler);

	gpio_open(&g_CS, GPIO_CS_PIN, 1);
	gpio_set(&g_CS, 1);
	gpio_open(&g_SDN, GPIO_SDN_PIN, 1);
	gpio_set(&g_SDN, 1);
	gpio_open(&g_IRQ, GPIO_NIRQ_PIN, 0);
	//input
	gpio_open(&g_GDO0, GPIO_00_PIN, 0);
	gpio_open(&g_GDO1, GPIO_01_PIN, 0);
	//output
	gpio_open(&g_GDO2, GPIO_02_PIN, 1);
	gpio_open(&g_GDO3, GPIO_03_PIN, 1);

	spidev_open(&g_spidev, NULL, SPI_MODE_0, 8, 1000000, 0); 

	g_input = buffer_create(64);

	if(NULL == g_input){
		return -1;
	}
	//
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );

	// Parse options
	while((opt = getopt(argc, argv, "m:h")) != -1) {
		switch(opt){
			case 'm':
				if(strcasecmp(optarg, "tx") == 0){
					mode = 0;
				}
				else if(strcasecmp(optarg, "rx") == 0){
					mode = 1;
				}
				break;
			case 'h':
				showusage();
				return 0;
				break;
		}
	}
	switch(mode){
		//tx mode
		case 0:
			gpio_set(&g_GDO2, 0);
			gpio_set(&g_GDO3, 1);
			SI446X_TXCONFIG();
			do{
				ch = getch();
				switch(ch){
				case '\r':
				case '\n':
					SI446X_SEND_HANDLE();
					usleep(500000);
					if( SI446X_GET_DEVICE_STATE( ) != 3 ){
						SI446X_TXCONFIG();
					}
					//printf("\r\n");
					printf("\n");
					break;
				default:
					printf("%c", ch);
					if(BUFFER_SIZE(g_input) < 64){
						buffer_append(g_input, ch);
					}
					else{
						printf("\r\n");
						BUFFER_RESET(g_input);
					}
					break;
				}
			}
			while(1);
			break;
		//rx mode
		case 1:
			gpio_set(&g_GDO2, 1);
			gpio_set(&g_GDO3, 0);
			SI446X_RXCONFIG();
			do{
				SI446X_RCV_HANDLE();
				usleep(500000);
				SI446X_RXCONFIG();
			}while(1);
			break;
	}
}
