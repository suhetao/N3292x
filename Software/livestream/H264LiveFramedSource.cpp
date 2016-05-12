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

#include "H264LiveFramedSource.hh"
#include <GroupsockHelper.hh> // for "gettimeofday()"

#include <sstream>
#include <Base64.hh>

void signalNewH264LiveFrameData(void *device);

H264LiveFramedSource* H264LiveFramedSource::createNew(UsageEnvironment& env, AVProfile profile)
{
	return new H264LiveFramedSource(env, profile);
}

EventTriggerId H264LiveFramedSource::eventTriggerId = 0;

unsigned H264LiveFramedSource::referenceCount = 0;

char H264LiveFramedSource::starcode[] = {0x00, 0x00, 0x00, 0x01};
char H264LiveFramedSource::_starcode[] = {0x00, 0x00, 0x01};

void* H264LiveFramedSource::H264LiveThread(void *arg)
{
	H264LiveFramedSource* ptr = (H264LiveFramedSource*)arg;
	v4l_device *vd = &ptr->m_V4LDevice;
	h264encoder_device *hd = &ptr->m_H264EncoderDevice;
	queue *q = ptr->m_pFrameQueue;
	AVProfile *profile = &ptr->m_AVProfile;
	AVFrame pict;
	uint32_t length = 0, planar = 0;
	uint8_t *bitstream = NULL;
	uint32_t ysize = profile->width * profile->height;
	
	bitstream = new uint8_t[ysize * 3 / 2];
	int32_t bFirst = True;

	if(!bitstream){
		return NULL;
	}
	while(!ptr->m_bQuit){
		v4l_grab_capture(vd);
		v4l_grab_sync(vd);
		v4l_grab_picture(vd);

		planar = v4l_get_h264_planar(vd);
		pict.data[0] = (uint8_t *)planar;
		pict.data[1] = (uint8_t *)(planar + ysize);
		pict.data[2] = (uint8_t *)(planar + ysize + (ysize >> 2));

		if(ptr->isRequest){
			//Tirgger to Encode bitstream by Driver handle
			length = h264encoder_encode(hd, bitstream, (void *)&pict);
			if(length == 0){
				break;
			}
			ptr->GetSPSandPPS(bitstream, length);
			if(bFirst){
				bFirst = False;
				//close
				h264encoder_close(hd);
				//reopen
				h264encoder_open(hd, (char*)DEFAULT_H264ENCODER_DEVICE);
			}
			queue_push(q, bitstream, length);
			signalNewH264LiveFrameData(ptr);
		}	
	}
	delete [] bitstream;
	return NULL;
}

H264LiveFramedSource::H264LiveFramedSource(UsageEnvironment& env, AVProfile profile)
	: FramedSource(env), m_AVProfile(profile), m_KeepStartcode(True)
{
	if (referenceCount == 0) {
		int ret = 0;
		// Any global initialization of the device would be done here
		memset((void*)&m_V4LDevice, 0, sizeof(v4l_device));
		memset((void*)&m_H264EncoderDevice, 0, sizeof(h264encoder_device));
		//
		memcpy((void*)&(m_H264EncoderDevice.profile), (void*)&m_AVProfile, sizeof(AVProfile));
		v4l_open(&m_V4LDevice, (char*)DEFAULT_V4L_DEVICE, m_AVProfile.width, m_AVProfile.height);
		h264encoder_open(&m_H264EncoderDevice, (char*)DEFAULT_H264ENCODER_DEVICE);

		m_bQuit = False;
		isRequest = true;

		m_pFrameQueue = queue_create();
		pthread_mutex_init(&m_Mutex, NULL);

		ret = pthread_create(&m_ThreadID, NULL, H264LiveFramedSource::H264LiveThread, (void *)this);
		if(!ret){
		}
	}
	++referenceCount;

	// Any instance-specific initialization of the device would be done here:

	// We arrange here for our "deliverFrame" member function to be called
	// whenever the next frame of data becomes available from the device.
	//
	// If the device can be accessed as a readable socket, then one easy way to do this is using a call to
	//     envir().taskScheduler().turnOnBackgroundReadHandling( ... )
	// (See examples of this call in the "liveMedia" directory.)
	//
	// If, however, the device *cannot* be accessed as a readable socket, then instead we can implement it using 'event triggers':
	// Create an 'event trigger' for this device (if it hasn't already been done):
	if (eventTriggerId == 0) {
		eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
	}
}

H264LiveFramedSource::~H264LiveFramedSource()
{
	// Any instance-specific 'destruction' (i.e., resetting) of the device would be done here:

	--referenceCount;
	if (referenceCount == 0) {
		// Any global 'destruction' (i.e., resetting) of the device would be done here:
		m_bQuit = True;
		pthread_join(m_ThreadID, NULL);
		pthread_mutex_destroy(&m_Mutex);

		v4l_close(&m_V4LDevice);
		h264encoder_close(&m_H264EncoderDevice);

		queue_destroy(m_pFrameQueue);

		// Reclaim our 'event trigger'
		envir().taskScheduler().deleteEventTrigger(eventTriggerId);
		eventTriggerId = 0;
	}
}

void H264LiveFramedSource::doGetNextFrame()
{
	// This function is called (by our 'downstream' object) when it asks for new data.

	// Note: If, for some reason, the source device stops being readable (e.g., it gets closed), then you do the following:
	if (0 /* the source stops being readable */) {
		handleClosure();
		return;
	}

	// If a new frame of data is immediately available to be delivered, then do this now:
	// a new frame of data is immediately available to be delivered
	if(queue_is_empty(m_pFrameQueue)){
		//deliverFrame();
		if(!isRequest){
			isRequest = true;
		}
	}
	// No new data is immediately available to be delivered.  We don't do anything more here.
	// Instead, our event trigger must be called (e.g., from a separate thread) when new data becomes available.
}

void H264LiveFramedSource::deliverFrame0(void* clientData)
{
	((H264LiveFramedSource*)clientData)->deliverFrame();
}

void H264LiveFramedSource::deliverFrame()
{
	// This function is called when new frame data is available from the device.
	// We deliver this data by copying it to the 'downstream' object, using the following parameters (class members):
	// 'in' parameters (these should *not* be modified by this function):
	//     fTo: The frame data is copied to this address.
	//         (Note that the variable "fTo" is *not* modified.  Instead,
	//          the frame data is copied to the address pointed to by "fTo".)
	//     fMaxSize: This is the maximum number of bytes that can be copied
	//         (If the actual frame is larger than this, then it should
	//          be truncated, and "fNumTruncatedBytes" set accordingly.)
	// 'out' parameters (these are modified by this function):
	//     fFrameSize: Should be set to the delivered frame size (<= fMaxSize).
	//     fNumTruncatedBytes: Should be set iff the delivered frame would have been
	//         bigger than "fMaxSize", in which case it's set to the number of bytes
	//         that have been omitted.
	//     fPresentationTime: Should be set to the frame's presentation time
	//         (seconds, microseconds).  This time must be aligned with 'wall-clock time' - i.e., the time that you would get
	//         by calling "gettimeofday()".
	//     fDurationInMicroseconds: Should be set to the frame's duration, if known.
	//         If, however, the device is a 'live source' (e.g., encoded from a camera or microphone), then we probably don't need
	//         to set this variable, because - in this case - data will never arrive 'early'.
	// Note the code below.

	
	if (!isCurrentlyAwaitingData()) return; // we're not ready for the data yet

	if(queue_is_empty(m_pFrameQueue)){
		isRequest = true;
		return;
	}
	u_int8_t* newFrameDataStart = NULL;
	unsigned int newFrameSize = 0;
	buffer *pbuf = NULL;
	queue_pop(m_pFrameQueue, &pbuf);

	newFrameDataStart = BUFFER_GET(pbuf);
	newFrameSize = BUFFER_SIZE(pbuf);

	// Deliver the data here:
	if (newFrameSize > fMaxSize) {
		fFrameSize = fMaxSize;
		fNumTruncatedBytes = newFrameSize - fMaxSize;
	} else {
		fFrameSize = newFrameSize;
	}
	gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
	// If the device is *not* a 'live source' (e.g., it comes instead from a file or buffer), then set "fDurationInMicroseconds" here.
	memmove(fTo, newFrameDataStart, fFrameSize);

	// After delivering the data, inform the reader that it is now available:
	FramedSource::afterGetting(this);
}

// extract a frame
uint8_t*  H264LiveFramedSource::ExtractFrame(uint8_t* frame, size_t& size, size_t& outsize)
{			
	uint8_t * outFrame = NULL;
	outsize = 0;
	uint32_t startcode_length = 0;
	m_FrameType = 0;

	if( (size >= 4) && (find_start_code(frame))){
		startcode_length = 4;
	}
	else if(_find_start_code(frame)){
		startcode_length = 3;
	}

	if (startcode_length != 0){
		m_FrameType = (frame[startcode_length] & 0x1F);
		uint8_t *ptr = (uint8_t*)memmem(&frame[startcode_length], size - startcode_length, starcode, 4);
		if (ptr == NULL){
			ptr = (uint8_t*)memmem(&frame[startcode_length], size - startcode_length, _starcode, 3);
		}
		if (m_KeepStartcode){
			outFrame = &frame[0];
		}
		else{
			size -=  startcode_length;
			outFrame = &frame[startcode_length];
		}
		if (NULL != ptr){
			outsize = ptr - outFrame;
		}
		else{
			outsize = size;
		}
		size -= outsize;
	}
	return outFrame;
}

// get sps and pps in idr frame
void H264LiveFramedSource::GetSPSandPPS(uint8_t* frame, uint32_t frameSize) 
{				
	size_t bufSize = frameSize;
	size_t size = 0;
	uint8_t* buffer = this->ExtractFrame(frame, bufSize, size);

	while (buffer != NULL){
		switch (m_FrameType){
			case 7:
				m_fSPS.assign((char*)buffer,size);
				break;
			case 8:
				m_fPPS.assign((char*)buffer,size);
				break;
			//P frame
			case 1:
				break;
			//I frame
			case 5: 
				if (!m_fSPS.empty() && !m_fPPS.empty()){

				}
			default:
				break;
		}
		if (m_fAuxLine.empty() && !m_fSPS.empty() && !m_fPPS.empty()){
			uint32_t profile_level_id = 0;					
			if (m_fSPS.size() >= 4) profile_level_id = (m_fSPS[1]<<16)|(m_fSPS[2]<<8)|m_fSPS[3]; 

			char* sps_base64 = base64Encode(m_fSPS.c_str(), m_fSPS.size());
			char* pps_base64 = base64Encode(m_fPPS.c_str(), m_fPPS.size());		

			std::ostringstream os; 
			os << "profile-level-id=" << std::hex << std::setw(6) << profile_level_id;
			os << ";sprop-parameter-sets=" << sps_base64 <<"," << pps_base64;
			m_fAuxLine.assign(os.str());

			delete [] sps_base64;
			delete [] pps_base64;
			break;
		}
		buffer = this->ExtractFrame(&buffer[size], bufSize, size);
	}
}

// The following code would be called to signal that a new frame of data has become available.
// This (unlike other "LIVE555 Streaming Media" library code) may be called from a separate thread.
// (Note, however, that "triggerEvent()" cannot be called with the same 'event trigger id' from different threads.
// Also, if you want to have multiple device threads, each one using a different 'event trigger id', then you will need
// to make "eventTriggerId" a non-static member variable of "H264LiveFramedSource".)
void signalNewH264LiveFrameData(void *device)
{
	H264LiveFramedSource* livesource  = (H264LiveFramedSource*)device;
	TaskScheduler* liveScheduler = &livesource->envir().taskScheduler();

	if (liveScheduler != NULL) { // sanity check
		liveScheduler->triggerEvent(H264LiveFramedSource::eventTriggerId, livesource);
	}
}
