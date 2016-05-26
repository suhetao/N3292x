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

#include "FastMath.h"
#include "mpu9250.h"

static void exithandler(int sig)
{
	MPU9250_Close();
	exit(-9);	
}

static void showusage()
{
	printf("mpu9250 [options]\n");
	printf("-m nor/dmp: normal or dmp mode\n");
	printf("-h : Help\n");
}

int main(int argc, char **argv)
{
	//mode 0 for normal, 1 for dmp
	int32_t opt, mode = 0;
	//
	int16_t acc[3], gyro[3], mag[3];
	float racc[3], rgyro[3], rmag[3];

	signal (SIGINT, exithandler);
	//signal (SIGQUIT, exithandler);
	//signal (SIGILL, exithandler);
	//signal (SIGABRT, exithandler);
	//signal (SIGFPE, exithandler);
	signal (SIGKILL, exithandler);
	//signal (SIGPIPE, exithandler);
	signal (SIGTERM, exithandler);

	MPU9250_Init();

	// Parse options
	while((opt = getopt(argc, argv, "m:h")) != -1) {
		switch(opt){
			case 'm':
				if(strcasecmp(optarg, "nor") == 0){
					mode = 0;
				}
				else if(strcasecmp(optarg, "dmp") == 0){
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
		case 0:
			do{
				usleep(200000);
				if (MPU9250_IsDataReady()){
					MPU9250_Get6AxisRawData(acc, gyro);
					rgyro[0] = DEGTORAD(gyro[0]) * 0.06097560975609756097560975609756f;
					rgyro[1] = DEGTORAD(gyro[1]) * 0.06097560975609756097560975609756f;
					rgyro[2] = DEGTORAD(gyro[2]) * 0.06097560975609756097560975609756f;
					
					racc[0] = acc[0] / 16384.0f;
					racc[1] = acc[1] / 16384.0f;
					racc[2] = acc[2] / 16384.0f;
					//
					printf("acceloremeter:%d,%d,%d\n", acc[0], acc[1], acc[2]);
				}
			}
			while(1);
			break;
		case 1:
			do{
			}while(1);
			break;
	}
}
