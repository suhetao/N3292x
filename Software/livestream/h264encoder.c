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

#include "h264encoder.h"

int h264encoder_open(h264encoder_device *hd, char *dev)
{
	AVProfile *profile = &(hd->profile);
	FAVC_ENC_PARAM *param = &(hd->param);
#ifdef RATE_CTL
	H264RateControl *rate = &(hd->rate);
#endif

	if(!dev){
		dev = DEFAULT_H264ENCODER_DEVICE;
	}
	if((hd->fd = open(dev, O_RDWR)) < 0){
		perror("open: ");
		return -1;
	}
	// Get Bitstream Buffer Information	
	if(ioctl(hd->fd, FAVC_IOCTL_ENCODE_GET_BSINFO, &(hd->workbuf)) < 0){
		close(hd->fd);
		perror("FAVC_IOCTL_ENCODE_GET_BSINFO: ");
		return -1;
	}
	hd->buf = mmap(NULL, (hd->workbuf).size, PROT_READ | PROT_WRITE, MAP_SHARED, hd->fd, (hd->workbuf).offset);
	if((void *)hd->buf == MAP_FAILED){
		perror("mmap: ");
		return -1;
	}
	//-----------------------------------------------
	//  Issue Encode parameter to driver handle
	//-----------------------------------------------
	memset(param, 0, sizeof(FAVC_ENC_PARAM));

	param->u32API_version = H264VER;
	param->u32FrameWidth = profile->width;
	param->u32FrameHeight = profile->height;
	param->fFrameRate = profile->framerate;
	param->u32IPInterval = profile->gop_size; //60, IPPPP.... I, next I frame interval
	param->u32MaxQuant = profile->qmax;
	param->u32MinQuant= profile->qmin;
	param->u32Quant = profile->quant; //32
	param->u32BitRate = profile->bit_rate;
	param->ssp_output = -1;
	param->intra = -1;

	param->bROIEnable = 0;
	param->u32ROIX = 0;
	param->u32ROIY = 0;
	param->u32ROIWidth = 0;
	param->u32ROIHeight = 0;

#ifdef RATE_CTL
	memset(rate, 0, sizeof(H264RateControl));
	H264RateControlInit(rate, param->u32BitRate, RC_DELAY_FACTOR, RC_AVERAGING_PERIOD, RC_BUFFER_SIZE_BITRATE, (int)param->fFrameRate, (int)param->u32MaxQuant, (int)param->u32MinQuant, (unsigned int)param->u32Quant, param->u32IPInterval);
#endif
	hd->quant = profile->quant;
	if (ioctl(hd->fd, FAVC_IOCTL_ENCODE_INIT, &(hd->param)) < 0){
		close(hd->fd);
		perror("FAVC_IOCTL_ENCODE_INIT: ");
		return -1;
	}
	return 0;
}

int h264encoder_close(h264encoder_device *hd)
{
	//---------------------------------
	//  Close driver handle
	//---------------------------------	
	if(hd->fd) {
		close(hd->fd);
	}
	hd->fd = 0;
	if(hd->buf){
		munmap((void *)hd->buf, hd->workbuf.size);
	}
	hd->buf = 0;
	return 0;
}

int h264encoder_encode(h264encoder_device *hd, uint8_t *frame, void *data)
{
	AVFrame *pict = (AVFrame *)data;
	FAVC_ENC_PARAM *param = &(hd->param);
	AVProfile *profile = &(hd->profile);
#ifdef RATE_CTL
	H264RateControl *rate = &(hd->rate);
#endif

	param->pu8YFrameBaseAddr = (uint8_t *)pict->data[0];   //input user continued virtual address (Y), Y=0 when NVOP
	param->pu8UFrameBaseAddr = (uint8_t *)pict->data[1];   //input user continued virtual address (U)
	param->pu8VFrameBaseAddr = (uint8_t *)pict->data[2];   //input user continued virtual address (V)

	param->bitstream = frame;  //output User Virtual address   
	param->ssp_output = -1;
	param->intra = -1;
	param->u32IPInterval = 0; // use default IPInterval that set in INIT

	param->u32Quant = hd->quant;
	param->bitstream_size = 0;

	if (ioctl(hd->fd, FAVC_IOCTL_ENCODE_FRAME, param) < 0){
		perror("FAVC_IOCTL_ENCODE_FRAME: ");
		return 0;
	}
#ifdef RATE_CTL
	if (param->keyframe == 0) {
		H264RateControlUpdate(rate, param->u32Quant, param->bitstream_size , 0);
	}
	else{
		H264RateControlUpdate(rate, param->u32Quant, param->bitstream_size , 1);
	}
	hd->quant = hd->rate.rtn_quant;
#endif
	profile->intra = param->keyframe;

	return param->bitstream_size;
}

