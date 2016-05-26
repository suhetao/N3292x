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

#ifndef _GPIO_H
#define _GPIO_H

#include <stdint.h>

/* 
 * Note :
 *   PORT NAME[PIN] = GPIO [id]	
 *
 *   PORTA[ 0]      = gpio[ 0x00]
 *   PORTA[ 1]      = gpio[ 0x01]	
 *                  :
 *   PORTA[31]      = gpio[ 0x1F]
 *                  :
 *   PORTB[ 0]      = gpio[ 0x20]
 *                  :
 *   PORTB[31]      = gpio[ 0x3F]
 *                  :
 *   PORTC[ 0]      = gpio[ 0x40]
 *                  :
 *   PORTC[31]      = gpio[ 0x5F]
 *                  :
 *   PORTD[ 0]      = gpio[ 0x60]
 *                  :
 *   PORTD[31]      = gpio[ 0x7F]
 *                  :
 *   PORTE[ 0]      = gpio[ 0x80]
 *                  :
 *   PORTE[31]      = gpio[ 0x9F]
 *                  :
 *   PORTG[ 0]      = gpio[ 0xA0]
 *                  :
 *   PORTG[31]      = gpio[ 0xBF]
 *                  :
 *   PORTH[ 0]      = gpio[ 0xC0]
 *                  :
 *                  :
 *   PORTH[31]      = gpio[ 0xDF]
 */
/*
OUTPUT:
SDN->GPC14 (10)
CS->GPG3
INPUT:
NIRQ->GPC12 (11)

GPIO3->GPC11 (13)
GPIO2->GPC13 (12)
GPIO1->GPC15 (8)
GPIO0->GPE0 (7)
*/

#define GPIO_EXPORT_DEVICE "/sys/class/gpio/export"
#define GPIO_UNEXPORT_DEVICE "/sys/class/gpio/unexport"

#define GPIO_SDN_PIN (0x4E)
#define GPIO_CS_PIN (0xA3)
#define GPIO_NIRQ_PIN (0x4C)
#define GPIO_00_PIN (0x80)
#define GPIO_01_PIN (0x4F)
#define GPIO_02_PIN (0x4D)
#define GPIO_03_PIN (0x4B)
#define GPIO_PATH_MAX_LENGTH (32)

typedef struct gpio_
{
	uint32_t pin;
	uint32_t direction;
	char path[GPIO_PATH_MAX_LENGTH];
	char value;
}gpio;

//val < 0 for input; val>0 output high; val==0 output low
int32_t gpio_open(gpio* g, int32_t pin, int32_t direction);
int32_t gpio_close(gpio* g);
// return < 0 for err; 0 for low; 1 for high;
int32_t gpio_get(gpio* g);
int32_t gpio_set(gpio* g, int32_t value);

#endif
