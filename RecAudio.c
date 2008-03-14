// This file is based on iphone syntehsis example
// author: shishir birmiwal ;  shishir.org
// this program records from standard input
//  
// Mar 14 08
//
//
//
// note: a lot of clean-up is reqd. this is a basic proof of concept code for recording audio
//
// iphone synthesis example. Uses an "AudioQueue" object.
// Is able to synth audio without killing mediaserverd, and volume is controllable via iphone softvolume
// headphone->speaker state is preserved. Other system audio plays on top of this audio.

// Brian Whitman - brian.whitman@variogr.am  - http://variogr.am/ - 9/2/07

// Please note that this code will not compile as it is missing headers. I do not have have these headers.
#include "PlayAudio-withoutAQ.h"


#define BUFFERS 3

// Callback info struct -- holds the queue and the buffers and some other stuff for the callback
typedef struct AQCallbackStruct {
	AudioQueueRef					queue;
	UInt32							frameCount;
	AudioQueueBufferRef				mBuffers[BUFFERS];
	AudioStreamBasicDescription		mDataFormat;
} AQCallbackStruct;


// synth params
int phaseL =0;
int phaseR = 0;
float FL = 0;
float FR = 0;
float amp = 0.5;

int timestamp_rec = 0;
FILE *f = NULL;


void callbackRehab(void * in, AudioQueueRef inQ, AudioQueueBufferRef outQB) { //used with rehabAudio()
	AudioQueueEnqueueBuffer(inQ, outQB, 0, NULL);
}


/** 
This method is called for two reasons.  
The first is to rememdy what I call an "Error B", which is the "not ready" error (-536870184).  
This will often occur when trying to start the input AudioQueue.  It will pause, and then return this error.  If it does so,
this method will fix it so that start can be called successfully.  Use the "false" parameter to accomplish this. (I haven't tested it with true...if true works, then eliminate the parameter and use true)
This method is also called to quench an extra thread that may be running in the mediaserver process.  It should be called right before exiting
application. ("true" parameter)
*/
void rehabAudio(bool aExiting) //called to remedy an "Error B"
{	
	UInt32 err;
	int i;
	AudioStreamBasicDescription zFormat;

	fprintf(stderr, "trying rehab!\n");
        zFormat.mSampleRate = 44100;
	zFormat.mFormatID = kAudioFormatLinearPCM;
	zFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger  | kAudioFormatFlagIsPacked;
	zFormat.mBytesPerPacket = 4;
	zFormat.mFramesPerPacket = 1; 
	zFormat.mBytesPerFrame = 4;
	zFormat.mChannelsPerFrame = 2;
	zFormat.mBitsPerChannel = 16;	
	
	AudioQueueRef zQueue;
        err = AudioQueueNewOutput(&zFormat, callbackRehab, NULL, CFRunLoopGetCurrent(), kCFRunLoopCommonModes, 0, &zQueue);
	
	if(err) printf("RehabAudioQueue: AudioQueueNewOutput err %d\n", err);
	
	if (aExiting==false)
	{
	err = AudioQueueStart(zQueue, NULL);
	if(err) printf("RehabAudioQueueStart err %d\n", err);
	}//end : not exiting
	
	
	
	err = AudioQueueDispose(zQueue, aExiting);
	 if(err) printf("RehabAudioQueueDispose err %d\n", err);
   


	
	
}//END: rehab

// Synthesis callback. Make your music here.
static void AQRecordCallback (
    void                                    *out,
    AudioQueueRef                           outQ,
    AudioQueueBufferRef                     outBuffer,
    const AudioTimeStamp                    *outStartTime,
    UInt32                                  outNumPackets,
    const AudioStreamPacketDescription      *outPacketDesc)
{
    AQCallbackStruct * outData = (AQCallbackStruct *)out;
    short *coreAudioBuffer = (short*) outBuffer->mAudioData;
    printf("cb - %d %d [%d] [%d]  ", outNumPackets, outData->mDataFormat.mBytesPerPacket,
		coreAudioBuffer[0], coreAudioBuffer[1]);
    if (outNumPackets == 0 && outData->mDataFormat.mBytesPerPacket != 0)
    {
	//fprintf(stderr, "iphone-sound: error -- rcvd 0 packets \n");
        outNumPackets = outBuffer->mAudioDataByteSize
			 / outData->mDataFormat.mBytesPerPacket;
	
    }

    {
	int i;
	timestamp_rec += outData->frameCount;
/*	
	for (i=0;i<10;i++)
	{	
		coreAudioBuffer[i] = 3200*i;
	}
*/
	/*
	gAudioControls.rec_strm->rec_cb(
		gAudioControls.rec_strm->user_data,
		gAudioControls.play_strm->timestamp_rec,
		coreAudioBuffer,
		outData->frameCount * 2 );
	*/
	if (f==NULL)
	{
		f = fopen("dump.raw","wb");
	}	
	if (f!=NULL)
	{
		printf("frameCount: %d\n", outData->frameCount);
		fwrite(coreAudioBuffer, outData->frameCount*4, 1, f);
	}
    }
    AudioQueueEnqueueBuffer ( outQ, outBuffer, 0, NULL );

    return;
}

void set_routing_policy()
//MeCCA_AudioRoutingPolicy::setRoutingPolicy("record.standard");
{
	int zReturnValue;
	//zReturnValue = _ZN24MeCCA_AudioRoutingPolicy16setRoutingPolicyEPKc("record.standard");	
	//printf("returnValue: %d\n", zReturnValue);

	//zReturnValue = _ZN24MeCCA_AudioRoutingPolicy16setRoutingPolicyEPKc("record.standard");	
	//printf("returnValue: %d\n", zReturnValue);

	//zReturnValue = _ZN24MeCCA_AudioRoutingPolicy16setRoutingPolicyEPKc("record.standard");	
	//printf("returnValue: %d\n", zReturnValue);
	printf("setting route!\n");

	zReturnValue = _ZN24MeCCA_AudioRoutingPolicy16setRoutingPolicyEPKc("recording.standard", "recording.standard", "recording.standard");	
	printf("returnValue: %d\n", zReturnValue);
	zReturnValue = _ZN24MeCCA_AudioRoutingPolicy16setRoutingPolicyEPKc("record.standard", "record.standard", "record.standard");	
	printf("returnValue: %d\n", zReturnValue);
}

int main (void) {
	AQCallbackStruct out;
	UInt32 err;
	double sampleRate = 44100.0;
	int i;

	fprintf(stderr, "creating\n");
	fflush(stderr);

	// 2 sine waves
	FL = (2.0 * 3.14159 * 440.0) / sampleRate;
	FR = (2.0 * 3.14159 * 880.0) / sampleRate;
	
	// Set up our audio format -- signed 
	// interleaved shorts (-32767 -> 32767), 16 bit mono
	// The iphone does not want to play back float32s.
	out.mDataFormat.mSampleRate = sampleRate;
	out.mDataFormat.mFormatID = kAudioFormatLinearPCM;
	out.mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger  | 
					kAudioFormatFlagIsPacked;
	out.mDataFormat.mBytesPerPacket = 4;
	out.mDataFormat.mFramesPerPacket = 1;  // this means each packet in the AQ has one samples, one for each channel -> 4 bytes/frame/packet
	out.mDataFormat.mBytesPerFrame = 4;
	out.mDataFormat.mChannelsPerFrame = 2;
	out.mDataFormat.mBitsPerChannel = 16;


	
	// Set up the output buffer callback on the current run loop
//	err = AudioQueueNewInput(&out.mDataFormat, AQRecordCallback, &out,
//		NULL, kCFRunLoopCommonModes, 0, &out.queue);

//	if(err) fprintf(stderr, "AudioQueueNewInput err %d\n", err);

//	if (err == -536870184)
	{
		
		//AudioQueueStop(out.queue, true);
		// This is how you kill it.. not that we ever got here.
		//err = AudioQueueDispose(out.queue, true);
		//set_routing_policy();

		err = AudioQueueNewInput(&out.mDataFormat, AQRecordCallback, &out,
			NULL, kCFRunLoopCommonModes, 0, &out.queue);
//		if (err)
//			rehabAudio(false);

		if (err)
		{
			fprintf(stderr, "new audioQueueNewInput error!\n");
	//		fprintf(stderr, "unable to rehab audio!\n");
		}
	}
	
	// Set the size and packet count of each buffer read. (e.g. "frameCount")
	out.frameCount = 1152;
	// Byte size is bytesPerFrame*frames (see above)
	UInt32 bufferBytes  = out.frameCount * out.mDataFormat.mBytesPerFrame;
		
	// alloc 3 buffers.
	printf("bufferBytes is %d\n", bufferBytes);

	for (i=0; i<BUFFERS; i++) {
		err = AudioQueueAllocateBuffer(out.queue, bufferBytes, &out.mBuffers[i]);
		if(err) fprintf(stderr, "AudioQueueAllocateBuffer [%d] err %d\n",i, err);
		// "Prime" by calling the callback once per buffer
		//AQRecordCallback (&out, out.queue, out.mBuffers[i]);
		AudioQueueEnqueueBuffer( out.queue, out.mBuffers[i], 0, NULL);
	}	
	
	// set the volume of the queue -- note that the volume knobs on the ipod / celestial also change this
	//err = AudioQueueSetParameter(out.queue, kAudioQueueParam_Volume, 1.0);
	//if(err) fprintf(stderr, "AudioQueueSetParameter err %d\n", err);

	rehabAudio(false);
	// Start the queue
	err = AudioQueueStart(out.queue, NULL);
	if(err) fprintf(stderr, "AudioQueueStart err %d\n", err);

	set_routing_policy();

	fprintf(stderr, "going into loop!\n");
	fflush(stderr);

	// Hang around forever...
	{
		int timer = 40;
		while(timer--) 
		{
			CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false);
			printf("-%d - ", timer); 
			if (timer%10==0) printf("\n");
		}
	}

	AudioQueueStop(out.queue, true);
	// This is how you kill it.. not that we ever got here.
	err = AudioQueueDispose(out.queue, true);
	
	if (f!=NULL)
	{
		fclose(f);
	}
    return 0;
}
