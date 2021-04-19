//Created with the help of different example files provided by PortAudio
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "portaudio.h"
#include "pa_linux_alsa.h"
#include "ANC.h"
#define FRAMES_PER_BUFFER (512)//was 512
#define CHANNEL_COUNT (2)
static paTestData data;
static PaDeviceInfo device;
int main(){
	printf("ANC PROGRAM STARTING\n");
	
	paTestData data;
	PaStreamParameters inputParameters, outputParameters;
	PaStream *stream;
	PaError err;
	int i, totalFrames, totalSamples, numberOfBytes;
	FILE *file;
	
	err = Pa_Initialize();
	if(err != paNoError) goto error;
	
	//Default input devices
	inputParameters.device = Pa_GetDefaultInputDevice();
	if(inputParameters.device == paNoDevice){ printf("Input device could not be found.\n"); goto error;}
	outputParameters.device = Pa_GetDefaultOutputDevice();
	if(outputParameters.device == paNoDevice){ printf("Output device could not be found.\n"); goto error;}
	
	//Setup devices
	inputParameters.sampleFormat = paFloat32;
	outputParameters.sampleFormat = paFloat32;
	inputParameters.channelCount = CHANNEL_COUNT;
	outputParameters.channelCount = CHANNEL_COUNT;
	inputParameters.hostApiSpecificStreamInfo = NULL;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
	
	//Initialize data
	data.frameIndex = 0;
	data.maxFrameIndex = 44100 * 5; //5 for seconds to record
	totalFrames = data.maxFrameIndex;
	totalSamples = totalFrames*CHANNEL_COUNT;
	numberOfBytes = sizeof(SAMPLE) * totalSamples;
	data.recordArray = (SAMPLE *) malloc( numberOfBytes );
	if(data.recordArray == NULL){ printf("\nrecordArray could not be initialized\n"); free(data.recordArray); return -1;}
	memset(data.recordArray, 0.0f, totalSamples);
  data.recordArrayInverse = (SAMPLE *) malloc( numberOfBytes );
	if(data.recordArrayInverse == NULL){ printf("\nrecordArrayInverse could not be initialized\n"); free(data.recordArrayInverse); return -1;}
	memset(data.recordArrayInverse, 0.0f, totalSamples);

  //INIT FILE
	file = fopen("_recordArrayInit.raw","wb");
	if(file == NULL){ printf("Could not open file '_recordArrayInit.raw'\n");}
	else {
		fwrite(data.recordArray, CHANNEL_COUNT * sizeof(SAMPLE),totalFrames, file);
		fclose(file);
		printf("Wrote raw empty audio data to '_recordArrayInit.raw'\n");
			
	}
 	file = fopen("_recordArrayInverseInit.raw","wb");
	if(file == NULL){ printf("Could not open file '_recordArrayInverseInit.raw'\n");}
	else {
		fwrite(data.recordArrayInverse, CHANNEL_COUNT * sizeof(SAMPLE),totalFrames, file);
		fclose(file);
		printf("Wrote raw empty audio data to '_recordArrayInverseInit.raw'\n");
			
	}
 
	// Open an audio I/O stream //
    printf("STARTING AUDIO STREAM\n");
    err = Pa_OpenStream( &stream,
                                &inputParameters,          
                                NULL,//&outputParameters,        
                                //paFloat32,  /* 32 bit floating point output */
                                44100,/* SAMPLE_RATE */
                                FRAMES_PER_BUFFER, /*FRAMES_PER_BUFFER, /* frames per buffer, i.e. the number
                                                   of sample frames that PortAudio will
                                                   request from the callback. Many apps
                                                   may want to use
                                                   paFramesPerBufferUnspecified, which
                                                   tells PortAudio to pick the best,
                                                   possibly changing, buffer size.*/
								paClipOff, //paClipOff
								paTestCallBack, /* this is your callback function paTestCallBack */
                                &data ); /*This is a pointer that will be passed to
                                                   your callback &data*/
    if(err != paNoError ) goto error;
    
	PaAlsa_EnableRealtimeScheduling(stream, 1);
	err = Pa_StartStream(stream); //calls the callback function until strem
	if(err != paNoError) goto error;
	
	//PROGRAM END//
	//Pa_Sleep(5*1000); //5 second sleep
	//printf("Enter any key to end the progam\n");
	//getchar();
	while(Pa_IsStreamActive(stream));
	
	err = Pa_StopStream(stream); //Pa_AbortStream() is an option too
	if(err != paNoError) goto error;
	err = Pa_CloseStream(stream);
	if(err != paNoError) goto error;
	
  //WRITE TO FILE
	file = fopen("recordArray.raw","wb");
	if(file == NULL){ printf("Could not open file\n");}
	else {
	  fwrite(data.recordArray, CHANNEL_COUNT * sizeof(SAMPLE),totalFrames, file);
		fclose(file);
		printf("Wrote raw audio data to 'recordArray.raw'\n");
			
	}
 	file = fopen("recordArrayInverse.raw","wb");
	if(file == NULL){ printf("Could not open file\n");}
	else {
	  fwrite(data.recordArrayInverse, CHANNEL_COUNT * sizeof(SAMPLE),totalFrames, file);
		fclose(file);
		printf("Wrote raw audio data to 'recordArrayInverse.raw'\n");
			
	}
 
 //ARRAY DATA DISPLAY
 printf("RAW DATA SAMPLE:\n");
 int value = 6;
	for(i = 0; i < value;i++){
     printf(PRINT_S_FORMAT, data.recordArray[i]);
 }
 printf("\nMODIFIED DATA SAMPLE:\n");
 for(i = 0; i < value;i++){
     printf(PRINT_S_FORMAT, data.recordArrayInverse[i]);
 }
 printf("\nCOMBINED DATA SAMPLES:\n");
 for(i = 0; i < value;i++){
     printf(PRINT_S_FORMAT, data.recordArray[i] + data.recordArrayInverse[i]);
 }
 printf("\n");
 
 
	free(data.recordArray);
  free(data.recordArrayInverse);
	err = Pa_Terminate();
	if(err != paNoError) goto error;
	
	printf("ENDING AUDIO STREAM\n");
	return 0;
	
error: //DEFAULT ERROR DISPLAY CODE
	if(stream){ Pa_AbortStream(stream); Pa_CloseStream(stream);}
	free(data.recordArray);
  free(data.recordArrayInverse);
	Pa_Terminate();
    fprintf( stderr, "An error occurred while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return -1;
}

static int paTestCallBack( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
	//DO NOT DO: memory allocation/deallocation, I/O (including file I/O as well as console I/O, such as printf()), 
	//context switching (such as exec() or yield()), mutex operations, or anything else that might rely on the OS
	(void) outputBuffer;
	(void) timeInfo;
	(void) statusFlags;
	(void) userData;
	
	int done = paContinue;
	paTestData *data = (paTestData*)userData;
  //SAMPLE *out = (SAMPLE*)outputBuffer; NOT NEEDED JUST YET
  const SAMPLE *readPointer = (const SAMPLE*)inputBuffer; //const
	SAMPLE *writePointerRecordArray = &data->recordArray[data -> frameIndex * CHANNEL_COUNT]; 
  SAMPLE *writePointerInverseArray = &data->recordArrayInverse[data -> frameIndex * CHANNEL_COUNT]; 
  SAMPLE *readPointerHolder;
	long i, framesToCalculate;
	unsigned long remainingFrames = (data -> maxFrameIndex) - (data -> frameIndex);
	
	if(remainingFrames < framesPerBuffer){
		framesToCalculate = remainingFrames;
		done = paComplete;
	}
	else{
		framesToCalculate = framesPerBuffer;
		done = paContinue;
	}
	
	if( inputBuffer == NULL){
		for(i = 0; i < framesToCalculate; i++){
				*writePointerRecordArray++ = SAMPLE_SILENCE;
        *writePointerInverseArray++ = SAMPLE_SILENCE;
				if(CHANNEL_COUNT==2){
            *writePointerRecordArray++ = SAMPLE_SILENCE; 
            *writePointerInverseArray++ = SAMPLE_SILENCE;
            }
		}
	}
	else{
   for(i = 0; i < framesToCalculate; i++){
        readPointerHolder = readPointer++;
				*writePointerRecordArray++  = *readPointerHolder;        //was *readPointer++; Problem- the ++ increments
        *writePointerInverseArray++ = (-1.0)*(*readPointerHolder);   
				if(CHANNEL_COUNT==2){
          readPointerHolder = readPointer++;
          *writePointerRecordArray++  = *readPointerHolder;  
          *writePointerInverseArray++ = (-1.0)*(*readPointerHolder);
          }
		}
	}
   	data->frameIndex = (data->frameIndex) + framesToCalculate;
    return done;
}



