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

#include "v4l.h"

int v4l_open(v4l_device* vd, char* dev, int width, int height)
{
	uint32_t i;

	if(!dev){
		dev = DEFAULT_V4L_DEVICE;
	}
	if((vd->fd = open(dev, O_RDWR)) < 0){
		perror("open: ");
		return -1;
	}
	if(ioctl(vd->fd, VIDIOCGCAP, &(vd->capability)) < 0){
		perror("VIDIOCGCAP: ");
		return -1;
	}
	if(ioctl(vd->fd, VIDIOCGPICT, &(vd->picture)) < 0){
		perror("VIDIOCGPICT: ");
		return -1;
	}
	//
	vd->picture.palette = VIDEO_PALETTE_YUV422;
	vd->picture.depth = 16;
	if(ioctl(vd->fd, VIDIOCSPICT, &(vd->picture)) < 0){
		perror("VIDIOCSPICT: ");
		return -1;
	}
	//
	if(ioctl(vd->fd, VIDIOCGMBUF, &(vd->mbuf)) < 0) {
		perror("VIDIOCGMBUF: ");
		return -1;
	}
	if((vd->buf = mmap(0, vd->mbuf.size, PROT_READ | PROT_WRITE, MAP_SHARED, vd->fd, 0)) < 0){
		if ((unsigned char*)-1 == vd->buf) {
			vd->buf = mmap(0, vd->mbuf.size, PROT_READ | PROT_WRITE, MAP_PRIVATE, vd->fd, 0);
			if ((unsigned char*)-1 == vd->buf) {
				perror("mmap: ");
				return -1;
			}
		}
	}
	if(v4l_grab_init(vd, 0, width, height, VIDEO_PALETTE_YUV420P_MACRO) < 0){
		fprintf(stderr, "v4l_grab_init failed!\n");
		return -1;
	}
	//10->OK, 4-> xOK, 3-> Fail
	for(i = 0; i < 2; i++){
		//start grab the first picture
		if(v4l_grab_sync(vd) < 0){
			fprintf(stderr, "v4l_grab_sync failed!\n");
			return -1;
		}
		if(v4l_grab_picture(vd) < 0){
			fprintf(stderr, "v4l_grab_picture failed!\n");
			return -1;
		}
		if(v4l_grab_capture(vd) < 0){
			fprintf(stderr, "v4l_grab_capture failed!\n");
			return -1;
		}
	}
	return 0;
}

int v4l_close(v4l_device* vd)
{
	if(vd->fd){
		close(vd->fd);
	}
	vd->fd = 0;
	if(vd->buf){
			munmap(vd->buf, vd->mbuf.size);
	}
	vd->buf = 0;
	return 0;
}


int v4l_grab_init(v4l_device* vd, int frame, int width, int height, int palette)
{
	if(v4l_grab_stop(vd) < 0){
		return -1;
	}
	vd->mmap.frame = frame;
	vd->mmap.format = palette;
	vd->mmap.height = height;
	vd->mmap.width = width;
	if(ioctl(vd->fd, VIDIOCMCAPTURE, &(vd->mmap)) < 0){
		perror("v4l_grab_init");
		return -1;
	}
	if(v4l_grab_start(vd) < 0){
		return -1;
	}
	vd->preframe = frame;
	//this is now the grabbing frame
	vd->frame = (frame + 1) % (vd->mbuf).frames;
	return 0;
}

int v4l_grab_stop(v4l_device* vd)
{
	uint32_t bstop = 1;
	if (ioctl(vd->fd, VIDIOCCAPTURE, &bstop) < 0) {
		perror("v4l_grab_stop: ");
		return -1;
	}
	return 0;
}

int v4l_grab_start(v4l_device* vd)
{
	uint32_t bstop = 0;
	if (ioctl(vd->fd, VIDIOCCAPTURE, &bstop) < 0) {
		perror("v4l_grab_start: ");
		return -1;
	}
	vd->frame = 0;
	return 0;
}

int v4l_grab_sync(v4l_device* vd)
{
	int32_t i = 0;
	while (ioctl(vd->fd, VIDIOCSYNC, &(vd->preframe)) < 0 && (errno == EAGAIN || errno == EINTR)){
		usleep(10000);
		i++;
		if(i >= 100){
			perror("v4l_grab_sync: ");
			return -1;
		}
	}
	return 0;
}

int v4l_grab_capture(v4l_device* vd)
{
	vd->mmap.frame = vd->frame;
	if(ioctl(vd->fd, VIDIOCMCAPTURE, &(vd->mmap)) < 0){
		perror("v4l_grab_sync: ");
		return -1;
	}
	//this is now the capture frame
	vd->frame = (vd->frame + 1) % (vd->mbuf).frames;
	return 0;
}

int v4l_grab_picture(v4l_device* vd)
{
	(vd->buffer).index = vd->preframe;
	if(ioctl(vd->fd, VIDIOCGCAPTIME, &vd->buffer) < 0){
		perror("VIDIOCGCAPTIME: ");
		return -1;
	}
	vd->ts = (uint64_t)vd->buffer.timestamp.tv_sec * 1000000 + (uint64_t)vd->buffer.timestamp.tv_usec;
	//encode physical address
	vd->h264planar = vd->buffer.m.userptr;
    
	if(vd->prevts != 0){
		if((vd->ts - vd->prevts) > 300000){
			fprintf(stderr, ": V4L get raw picture over %"PRId64" us\n", (vd->ts - vd->prevts));
		}			
	}
	vd->prevts = vd->ts;
	vd->planar = vd->buf + vd->mbuf.offsets[vd->preframe];
	//this is now the sync frame
	vd->preframe = (vd->preframe + 1) % (vd->mbuf).frames;
	return 0;
}

