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

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

#include "H264VideoLiveServerMediaSubsession.hh"
#include "H264LiveFramedSource.hh"

TaskScheduler* scheduler = NULL;
UsageEnvironment* env = NULL;
//create rtsp
RTSPServer* server = NULL;
StreamReplicator*  replicator = NULL;
//
H264LiveFramedSource* livesource = NULL;
AVProfile profile;
//
ServerMediaSubsession * videoSubsession = NULL;

// To make the second and subsequent client for each stream reuse the same
// input stream as the first client (rather than playing the file from the
// start for each client), change the following "False" to "True":
Boolean reuseFirstSource = True;//False;

// add an RTSP session
static void addSession(RTSPServer* server, const char* sessionName, ServerMediaSubsession *subSession, ServerMediaSubsession *audio_subSession)
{
	UsageEnvironment& env(server->envir());
	ServerMediaSession* sms = ServerMediaSession::createNew(env, sessionName);
	sms->addSubsession(subSession);
	
	if (audio_subSession){
		sms->addSubsession(audio_subSession);
	}

	server->addServerMediaSession(sms);
	char* url = server->rtspURL(sms);
	fprintf(stderr, "lay this stream using the URL: \"%s\"\n", url );
	delete[] url;
}

int main(int argc, char** argv)
{
	// Begin by setting up our usage environment:
	scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);

	// Create the RTSP server:
	server = RTSPServer::createNew(*env, 554);
	if (server == NULL) {
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		exit(1);
	}

	//set encode parameter
	h264encoder_para_init(&profile);
	//create frame source
	livesource = H264LiveFramedSource::createNew(*env, profile);
	replicator = StreamReplicator::createNew(*env, livesource, false);
	//create video session
	videoSubsession = H264VideoLiveServerMediaSubsession::createNew(*env, reuseFirstSource, replicator);
	// Create Server Session
	addSession(server, "live", videoSubsession, NULL);
	//main loop
	env->taskScheduler().doEventLoop(); // does not return

	return 0; // only to prevent compiler warning
}
