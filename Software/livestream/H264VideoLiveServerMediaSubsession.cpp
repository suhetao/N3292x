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

#include "H264VideoLiveServerMediaSubsession.hh"
#include "H264VideoRTPSink.hh"
#include "H264LiveFramedSource.hh"
#include "H264VideoStreamFramer.hh"
//#include "H264VideoStreamDiscreteFramer.hh"

//#include <string>
#include <sstream>
//#include <iostream>

static void checkForAuxSDPLine(void* clientData) 
{
	H264VideoLiveServerMediaSubsession* subsess = (H264VideoLiveServerMediaSubsession*)clientData;
	subsess->checkForAuxSDPLine1();
}

char *H264VideoLiveServerMediaSubsession::getAuxLine1(H264LiveFramedSource* source)
{
	char* auxLine = NULL;
	std::string auxLine1;

	if(source){
		auxLine1 = source->getAuxLine();
		if(auxLine1.empty()){
			return NULL;
		}
		std::ostringstream os; 
		os << "a=fmtp:96 ";				
		os << auxLine1;				
		os << "\r\n";				
		auxLine = strdup(os.str().c_str());
	}
	return auxLine;
}

void H264VideoLiveServerMediaSubsession::checkForAuxSDPLine1() 
{
	H264LiveFramedSource* liveSource = NULL;

	if(fAuxSDPLine != NULL){
		// Signal the event loop that we're done:
		setDoneFlag();
	}
	else if(NULL != m_Replicator && NULL != (liveSource = (H264LiveFramedSource*)m_Replicator->inputSource())
		&& NULL != (fAuxSDPLine = this->getAuxLine1(liveSource))){
			// Signal the event loop that we're done:
			setDoneFlag();
	}
	else if(!fDoneFlag){
		// try again after a brief delay:
		double delay = 10;  // ms  
		int uSecsToDelay = delay * 1000;  // us  
		nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)checkForAuxSDPLine, this);
	}
}

H264VideoLiveServerMediaSubsession* H264VideoLiveServerMediaSubsession::createNew(UsageEnvironment& env, Boolean reuseFirstSource, StreamReplicator* replicator)
{
	return new H264VideoLiveServerMediaSubsession(env, reuseFirstSource, replicator);
}

H264VideoLiveServerMediaSubsession::H264VideoLiveServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource, StreamReplicator* replicator)
: OnDemandServerMediaSubsession(env, reuseFirstSource)
{
	fSDPLines = NULL;
	fAuxSDPLine = NULL;
	fDoneFlag = 0;
	m_Replicator = replicator;
}

H264VideoLiveServerMediaSubsession::~H264VideoLiveServerMediaSubsession()
{
	delete[] fSDPLines;
	delete[] fAuxSDPLine;
}

char const* H264VideoLiveServerMediaSubsession::getAuxSDPLine()
{
	if (fAuxSDPLine != NULL){
		// it's already been set up (for a previous client)
		return fAuxSDPLine; 
	}
	else{
		// Check whether the sink's 'auxSDPLine()' is ready:
		checkForAuxSDPLine(this);
		envir().taskScheduler().doEventLoop(&fDoneFlag);
	}
	return fAuxSDPLine;
}

FramedSource* H264VideoLiveServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate)
{
	estBitrate = 90000; // kbps, estimate

	// Create the video source:
	FramedSource* liveSource = m_Replicator->createStreamReplica();
	if (liveSource == NULL){
		return NULL;
	}

	// Create a framer for the Video Elementary Stream:
	//return H264VideoStreamDiscreteFramer::createNew(envir(), liveSource);
	return H264VideoStreamFramer::createNew(envir(), liveSource);
}

RTPSink* H264VideoLiveServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/)
{
	return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

void H264VideoLiveServerMediaSubsession::setSDPLines()
{
	#if 1
	unsigned estBitrate = 90000;
	char const* mediaType = "video";
	unsigned char rtpPayloadType = 96;
	AddressString ipAddressStr(fServerAddressForSDP);
	char* rtpmapLine = strdup("a=rtpmap:96 H264/90000\n");
	char const* auxSDPLine = getAuxSDPLine();

	if (auxSDPLine == NULL){
		auxSDPLine = "";
	}

	char const* const sdpFmt =
		"m=%s %u RTP/AVP %d\r\n"
		"c=IN IP4 %s\r\n"
		"b=AS:%u\r\n"
		"%s"
		"%s"
		"a=control:%s\r\n";
	unsigned sdpFmtSize = strlen(sdpFmt)
		+ strlen(mediaType) + 5 /* max short len */ + 3 /* max char len */
		+ strlen(ipAddressStr.val())
		+ 20 /* max int len */
		+ strlen(rtpmapLine)
		+ strlen(auxSDPLine)
		+ strlen(trackId());
	char* sdpLines = new char[sdpFmtSize];
	sprintf(sdpLines, sdpFmt,
		mediaType, // m= <media>
		fPortNumForSDP, // m= <port>
		rtpPayloadType, // m= <fmt list>
		ipAddressStr.val(), // c= address
		estBitrate, // b=AS:<bandwidth>
		rtpmapLine, // a=rtpmap:... (if present)
		auxSDPLine, // optional extra SDP line
		trackId()); // a=control:<track-id>
	delete[] rtpmapLine;
	#else
	char const* sdpLines =
		"m=video 0 RTP/AVP 96\r\n"
		"c=IN IP4 0.0.0.0\r\n"
		"b=AS:500\r\n"
		"a=rtpmap:96 H264/90000\r\n"
		"a=fmtp:96 packetization-mode=1;profile-level-id=42001F;sprop-parameter-sets=Z0IAH+kAoAty,aM44gA==\r\n"
		"a=control:track1\r\n";
	#endif

	fSDPLines = strDup(sdpLines);

	//delete[] sdpLines;
}

char const* H264VideoLiveServerMediaSubsession::sdpLines() {
	if (fSDPLines == NULL) {
		setSDPLines();
	}
	return fSDPLines;
}