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

#include <stdlib.h>
#include "spidev.h"

int32_t spidev_open(spidev* sd, char* dev, uint8_t mode, uint8_t bits, uint32_t speed, uint16_t delay)
{
	uint32_t i = 0;

	if(!dev){
		dev = DEFALUT_SPIDEV_DEVICE;
	}
	if((sd->fd = open(dev, O_RDWR)) < 0){
		perror("open: ");
		return -1;
	}
	//spi mode
	if(ioctl(sd->fd, SPI_IOC_WR_MODE, &mode) < 0){
		perror("can't set spi mode: ");
		return -1;
	}
	if (ioctl(sd->fd, SPI_IOC_RD_MODE, &sd->mode) < 0){
		perror("can't get spi mode: ");
		return -1;
	}
	//bits per word
	if (ioctl(sd->fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0){
		perror("can't set bits per word: ");
		return -1;
	}
	if (ioctl(sd->fd, SPI_IOC_RD_BITS_PER_WORD, &sd->bits) < 0){
		perror("can't get bits per word: ");
		return -1;
	}
	//max speed hz
	if (ioctl(sd->fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0){
		perror("can't set max speed hz: ");
		return -1;
	}
	if (ioctl(sd->fd, SPI_IOC_RD_MAX_SPEED_HZ, &sd->speed) < 0){
		perror("can't get max speed hz: ");
		return -1;
	}
	for(i = 0; i < 3; i++){
		sd->xfer[i].tx_buf = (unsigned long)NULL;
		sd->xfer[i].rx_buf = (unsigned long)NULL;
		sd->xfer[i].delay_usecs = delay;
		sd->xfer[i].speed_hz = speed;
		sd->xfer[i].bits_per_word = bits;
	}

	return 0;
}

int32_t spidev_close(spidev* sd)
{
	if(0 < sd->fd){
		close(sd->fd);
		sd->fd = -1;
	}
	return 0;
}

int32_t spidev_writes(spidev* sd, uint8_t* buf, uint16_t size)
{
	sd->xfer[0].tx_buf = (unsigned long)buf;
	sd->xfer[0].len = size;

	if (ioctl(sd->fd, SPI_IOC_MESSAGE(1), &sd->xfer[0]) < 1){
		perror("can't send spi message: ");
		return -1;
	}
	return 0;
}

int32_t spidev_reads(spidev* sd, uint8_t* buf, uint16_t size)
{
	sd->xfer[1].rx_buf = (unsigned long)buf;
	sd->xfer[1].len = size;

	if (ioctl(sd->fd, SPI_IOC_MESSAGE(1), &sd->xfer[1]) < 1){
		perror("can't send spi message: ");
		return -1;
	}
	return 0;
}

uint8_t spidev_write_read_duplex(spidev* sd, uint8_t input)
{
	sd->xfer[2].tx_buf = (unsigned long)sd->tx_buf;
	sd->tx_buf[0] = input;
	sd->xfer[2].rx_buf = (unsigned long)sd->rx_buf;
	sd->rx_buf[0] = 0xFF;
	sd->xfer[2].len = 1;

	if (ioctl(sd->fd, SPI_IOC_MESSAGE(1), &sd->xfer[2]) < 1){
		perror("can't send spi message");
		abort();
	}
	return sd->rx_buf[0];
}

uint8_t spidev_write_then_read(spidev* sd, uint8_t *tx_buf, uint16_t tx_size, uint8_t *rx_buf, uint16_t rx_size)
{
	sd->xfer[0].tx_buf = (unsigned long)tx_buf;
	sd->xfer[0].len = tx_size;

	sd->xfer[1].rx_buf = (unsigned long)rx_buf;
	sd->xfer[1].len = rx_size;

	if (ioctl(sd->fd, SPI_IOC_MESSAGE(2), sd->xfer) < 1){
		perror("can't send spi message");
		return -1;
	}
	return 0;
}