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
#include <errno.h>
#include <string.h>

#include "gpio.h"

int32_t gpio_open(gpio* g, int32_t pin, int32_t direction)
{
	FILE * fp;
	char num[8];
	char path[32]="/sys/class/gpio/gpio";

	if(pin < 0){
		printf("gpio_open: %d error!\n", pin);
		return -1;
	}
	//export
	if((fp = fopen(GPIO_EXPORT_DEVICE,"w")) == NULL){
		perror("fopen: ");
		return -1;
	}
	//set pin
	g->pin = pin;
	fprintf(fp, "%d", pin);
	fclose(fp);
	//set direction
	sprintf(num, "%d", pin);
	strcat(path, num);
	strcat(path, "/direction");
	if((fp = fopen(path, "rb+")) == NULL){
		perror("fopen: ");
		return -1;
	}
	g->direction = direction;
	if(direction <= 0){
		fprintf(fp, "in");
	}
	else{
		fprintf(fp, "out");
	}
	fclose(fp);
	//
	strcpy(g->path, "/sys/class/gpio/gpio");
	sprintf(g->path + strlen(g->path),"%d/value",g->pin);
	return 0;
}

int32_t gpio_close(gpio* g)
{
	FILE * fp;

	if(g->pin < 0){
		printf("gpio_close: %d error!\n", g->pin);
		return -1;
	}
	if((fp = fopen(GPIO_UNEXPORT_DEVICE,"w")) == NULL){
		perror("fopen: ");
		return -1;
	}
	fprintf(fp,"%d",g->pin);
	fclose(fp);
	return 0;	
}

int32_t gpio_get(gpio* g)
{
	FILE * fp;

	if(g->pin < 0){
		printf("gpio_get: %d error!\n", g->pin);
		return -1;
	}
	if((fp = fopen(g->path, "rb")) == NULL){
		perror("fopen: ");
		return -1;
	}
	g->value = fgetc(fp);
	//fread(&g->value, sizeof(char), 1, fp);
	fclose(fp);
	return g->value == '0' ? 0 : 1;					
}

int32_t gpio_set(gpio* g, int32_t value)
{
	FILE * fp;

	if(g->pin < 0){
		printf("gpio_set: %d error!\n", g->pin);
		return -1;
	}
	if((fp = fopen(g->path, "rb+")) == NULL){
		perror("fopen: ");
		return -1;
	}
	fputc(value ? '1' : '0', fp);
	fclose(fp);
	return 0;
}
