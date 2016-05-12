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

#ifndef _H264_VIDEO_LIVE_SERVER_MEDIA_SUBSESSION_HH
#define _H264_VIDEO_LIVE_SERVER_MEDIA_SUBSESSION_HH

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif

#ifndef _STREAM_REPLICATOR_HH
#include "StreamReplicator.hh"
#endif

#include "H264LiveFramedSource.hh"


class H264VideoLiveServerMediaSubsession: public OnDemandServerMediaSubsession {
public:
	static H264VideoLiveServerMediaSubsession* createNew(UsageEnvironment& env, Boolean reuseFirstSource, StreamReplicator* replicator);

	// Used to implement "getAuxSDPLine()":
	void checkForAuxSDPLine1();
	char *getAuxLine1(H264LiveFramedSource *source);

protected:
	H264VideoLiveServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource, StreamReplicator* replicator);
	// called only by createNew();
	virtual ~H264VideoLiveServerMediaSubsession();

	virtual char const* sdpLines();

	void setDoneFlag() { fDoneFlag = ~0; }

protected: // redefined virtual functions
	//virtual char const* getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource);
	virtual char const* getAuxSDPLine();
	virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
	virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);

private:
	// used to implement "sdpLines()"
	void setSDPLines();

protected:
	char* fSDPLines;
	StreamReplicator* m_Replicator;

private:
	char* fAuxSDPLine;
	// used when setting up "fAuxSDPLine"
	char fDoneFlag;
};

#endif
