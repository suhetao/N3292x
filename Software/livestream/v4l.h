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

#ifndef _V4L_H_
#define _V4L_H_

#include <stdio.h>  
#include <stdlib.h> //stdio.h and stdlib.h are needed by perror function  
#include <sys/stat.h>  
#include <sys/types.h>  
#include <fcntl.h> //O_RDWR
#include <stdint.h> //uint32_t 
#include <unistd.h>  
#include <sys/mman.h> //unistd.h and sys/mman.h are needed by mmap function  
#include <stdbool.h>//false and true  
#include <sys/ioctl.h>  
#include <linux/videodev.h>//v4l API
#include <inttypes.h> //PRId64
#include <errno.h> //perror

#define VIDEO_PALETTE_YUV420P_MACRO 50 //YUV 420 Planar Macro

#define DEFAULT_V4L_DEVICE "/dev/video0"
#define DEFAULT_V4L_GRAB_WIDTH 1280
#define DEFAULT_V4L_GRAB_HEIGHT 720

#define VIDIOCGCAPTIME	_IOR('v',30,struct v4l2_buffer) //Get Capture time
#define VIDIOCSPREVIEW IOR('v',43, int)

typedef struct _v4l_struct
{
	int fd;
	struct video_capability capability;
	struct video_picture picture;
	struct video_mmap mmap;
	struct video_mbuf mbuf;
	uint8_t* buf;
	int frame;
	int preframe;
	int framesize;
	//fps control
	struct v4l2_buffer buffer;
	uint64_t prevts;
	uint64_t ts;
	//thread
	pthread_t thread;
	int32_t usethread;
	//
	uint8_t *planar;
	uint32_t h264planar;
}v4l_device;

static inline uint32_t v4l_get_h264_planar(v4l_device* vd)
{
	return vd->h264planar;
}

int v4l_open(v4l_device* vd, char* dev, int width, int height);
int v4l_grab_init(v4l_device* vd, int frame, int width, int height, int palette);
int v4l_grab_start(v4l_device* vd);
int v4l_grab_stop(v4l_device* vd);

int v4l_close(v4l_device* vd);

int v4l_grab_capture(v4l_device* vd);
int v4l_grab_sync(v4l_device* vd);
int v4l_grab_picture(v4l_device* vd);

#endif

