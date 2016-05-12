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

#ifndef _H264ENCODER_DEFINE_H_
#define _H264ENCODER_DEFINE_H_

#include <stdio.h>  
#include <stdlib.h> //stdio.h and stdlib.h are needed by perror function  
#include <sys/stat.h>  
#include <sys/types.h>  
#include <fcntl.h> //O_RDWR
#include <stdint.h> //uint32_t 
#include <string.h> //memset
#include <unistd.h>  
#include <sys/mman.h> //unistd.h and sys/mman.h are needed by mmap function  
#include <stdbool.h>//false and true  
#include <sys/ioctl.h>

#include "favc_version.h"
#include "favc_avcodec.h"
#include "ratecontrol.h"

//-------------------------------------
// Encoder part
//-------------------------------------
#define DEFAULT_H264ENCODER_DEVICE "/dev/w55fa92_264enc"
#define DEFAULT_H264ENCODER_WIDTH (1280)
#define DEFAULT_H264ENCODER_HEIGHT (720)

#define RATE_CTL
#define FIX_QUANT 0
#define IPInterval 31

//-------------------------------------
// Data structure
//-------------------------------------
typedef struct AVFrame {
	uint8_t *data[4];
} AVFrame;

typedef struct AVProfile
{
	uint32_t bit_rate;
	uint32_t width;   //length per dma buffer
	uint32_t height;
	uint32_t framerate;
	uint32_t frame_rate_base;
	uint32_t gop_size;
	uint32_t qmax;
	uint32_t qmin;
	uint32_t quant;
	uint32_t intra;
	AVFrame *coded_frame;
	char *priv;
} AVProfile;

typedef struct _h264encoder_struct
{
	int fd;
	AVProfile profile;
	FAVC_ENC_PARAM param;
	avc_workbuf_t workbuf;
	uint8_t* buf;
#ifdef RATE_CTL
	H264RateControl rate;
#endif
	int quant;
}h264encoder_device;

//set encode parameter
__inline void h264encoder_para_init(AVProfile *profile)
{
	memset((void*)profile, 0, sizeof(AVProfile));
	//set the default value
	profile->qmax = 51;
	profile->qmin = 0;
#ifdef RATE_CTL	
	profile->quant = 26;// Init Quant
#else
	profile->quant = FIX_QUANT;
#endif	
	profile->bit_rate = 512000;
	profile->width = DEFAULT_H264ENCODER_WIDTH;
	profile->height = DEFAULT_H264ENCODER_HEIGHT;
	profile->framerate = 30;
	profile->frame_rate_base = 1;
	profile->gop_size = IPInterval;
}

int h264encoder_open(h264encoder_device *hd, char* dev);
int h264encoder_encode(h264encoder_device *hd, uint8_t *frame, void *data);
int h264encoder_close(h264encoder_device *hd);

#endif
//#ifndef _H264_DEFINE_H_
