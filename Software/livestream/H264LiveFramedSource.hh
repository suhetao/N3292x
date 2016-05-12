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

#ifndef _H264LIVE_FRAMEDSOURCE_HH
#define _H264LIVE_FRAMEDSOURCE_HH

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif

#include <string>
#include <list>
#include <iostream>
#include <iomanip>

#ifdef __cplusplus
extern "C" {
#endif

#include "v4l.h"
#include "h264encoder.h"
#include "queue.h"

#ifdef __cplusplus
}
#endif

__inline int32_t find_start_code(uint8_t *argv){
	return ((0x00 != argv[0]) || (0x00 != argv[1]) || (0x00 != argv[2]) || (0x01 != argv[3]))?false:true;
}

__inline int32_t _find_start_code(uint8_t *argv){
	return ((0x00 != argv[0]) || (0x00 != argv[1]) || (0x01 != argv[2]))?false:true;
}

class H264LiveFramedSource: public FramedSource
{
public:
	static H264LiveFramedSource* createNew(UsageEnvironment& env, AVProfile profile);
	static void* H264LiveThread(void *arg);

	std::string getAuxLine() { return m_fAuxLine; };

public:
	static EventTriggerId eventTriggerId;
	// Note that this is defined here to be a static class variable, because this code is intended to illustrate how to
	// encapsulate a *single* device - not a set of devices.
	// You can, however, redefine this to be a non-static member variable.

	static char starcode[4];
	static char _starcode[3];

	uint8_t* ExtractFrame(uint8_t* frame, size_t& size, size_t& outsize);
	void GetSPSandPPS(uint8_t* frame, uint32_t frameSize);

protected:
	H264LiveFramedSource(UsageEnvironment& env, AVProfile profile);
	// called only by createNew(), or by subclass constructors
	virtual ~H264LiveFramedSource();

private:
	// redefined virtual functions:
	virtual void doGetNextFrame();
	//virtual void doStopGettingFrames(); // optional

private:
	static void deliverFrame0(void* clientData);
	void deliverFrame();

protected:
	std::string m_fAuxLine;
	std::string m_fSPS;
	std::string m_fPPS;

private:
	static unsigned referenceCount; // used to count how many instances of this class currently exist
	//
	v4l_device m_V4LDevice;
	h264encoder_device m_H264EncoderDevice;
	AVProfile m_AVProfile;
	pthread_t m_ThreadID;

	queue *m_pFrameQueue;
	pthread_mutex_t m_Mutex;
	
	int32_t m_bQuit;
	int32_t isRequest;
	//
	int32_t m_FrameType;
	int32_t m_KeepStartcode;

};

#endif
