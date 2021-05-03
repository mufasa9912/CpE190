//Created with the help of different example files provided by PortAudio
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "portaudio.h"
#include "pa_linux_alsa.h"
#include "ANC_Filter.h"
#define FRAMES_PER_BUFFER (4096)//was 512
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
  data.outputSilenceFlag = 0;
  data.freqEstimate = -1;
	data.frameIndex = 0;
  data.freqIndex = 0;
	data.maxFrameIndex = 44100 * 5; //5 for seconds to record
	totalFrames = data.maxFrameIndex;
	totalSamples = totalFrames*CHANNEL_COUNT;
	numberOfBytes = sizeof(SAMPLE) * totalSamples;
	data.recordArray = (SAMPLE *) malloc( numberOfBytes );
	if(data.recordArray == NULL){ printf("\nrecordArray could not be initialized\n"); free(data.recordArray); return -1;}
	memset(data.recordArray, 0.0f, totalSamples);
  data.recordArrayFilterOutput = (SAMPLE *) malloc( numberOfBytes );
	if(data.recordArrayFilterOutput == NULL){ printf("\nrecordArrayFilterOutput could not be initialized\n"); free(data.recordArrayFilterOutput); return -1;}
	memset(data.recordArrayFilterOutput, 0.0f, totalSamples);
  data.freqArray = (SAMPLE *) malloc( numberOfBytes/FRAMES_PER_BUFFER );//probably
	if(data.freqArray == NULL){ printf("\nfreqArray could not be initialized\n"); free(data.freqArray); return -1;}
	memset(data.freqArray, 0.0f, totalSamples/(2*FRAMES_PER_BUFFER));
  
  //INIT FILE
	file = fopen("_recordArrayInit.raw","wb");
	if(file == NULL){ printf("Could not open file '_recordArrayInit.raw'\n");}
	else {
		fwrite(data.recordArray, CHANNEL_COUNT * sizeof(SAMPLE),totalFrames, file);
		fclose(file);
		printf("\nWrote raw empty audio data to '_recordArrayInit.raw'\n");
			
	}
 	file = fopen("_recordArrayFilterOutputInit.raw","wb");
	if(file == NULL){ printf("Could not open file '_recordArrayFilterOutputInit.raw'\n");}
	else {
		fwrite(data.recordArrayFilterOutput, CHANNEL_COUNT * sizeof(SAMPLE),totalFrames, file);
		fclose(file);
		printf("Wrote raw empty audio data to '_recordArrayFilterOutputInit.raw'\n");
			
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

  while(Pa_IsStreamActive(stream)){
    //printf("%.2f Hz\n", data.freqEstimate);
  }

 	//PROGRAM END//
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
 	file = fopen("recordArrayFilterOutput.raw","wb");
	if(file == NULL){ printf("Could not open file\n");}
	else {
	  fwrite(data.recordArrayFilterOutput, CHANNEL_COUNT * sizeof(SAMPLE),totalFrames, file);
		fclose(file);
		printf("Wrote filtered audio data to 'recordArrayFilterOutput.raw'\n");
	}
  int ohboy;
  file = fopen("FreqOutput.csv","wb");
  if(file == NULL){ printf("Could not open file\n");}
  else{
    fprintf(file,"Index, Frequency\n");
    for(ohboy = 0; ohboy < totalFrames/FRAMES_PER_BUFFER; ohboy++){
      fprintf(file,"%d, %lf\n", ohboy, data.freqArray[ohboy]);
    }
    fclose(file);
    printf("Wrote frequency data to 'FreqOutput.csv'\n");
  }
  file = fopen("Amplitudes.csv","wb");
  if(file == NULL){ printf("Could not open file\n");}
  else{
    fprintf(file,"Index, InputAmp, FilteredAmp\n");
    for(ohboy = 0; ohboy < totalSamples/2; ohboy++){
      fprintf(file,"%d, %lf, %lf\n", ohboy, data.recordArray[2*ohboy], data.recordArrayFilterOutput[2*ohboy]);
    }
    fclose(file);
    printf("Wrote amplitude data to 'Amplitudes.csv'\n");
  }
 
 //ARRAY DATA DISPLAY
 printf("%.20f ", data.freqEstimate);
 printf("RAW DATA SAMPLE:\n");
 int value = 60;
	for(i = 0; i < value;i++){
    printf(PRINT_S_FORMAT, data.recordArray[i]);
 }
printf("\n");
 printf("FREQUENCY ARRAY DATA SAMPLE:\n");
 value = 20;
	for(i = 0; i < value;i++){
    printf(PRINT_S_FORMAT, data.freqArray[i]);
 }

	free(data.recordArray);
  free(data.recordArrayFilterOutput);
  free(data.freqArray);
	err = Pa_Terminate();
	if(err != paNoError) goto error;
	
	printf("\nENDING AUDIO STREAM\n");
	return 0;
	
error: //DEFAULT ERROR DISPLAY CODE
	if(stream){ Pa_AbortStream(stream); Pa_CloseStream(stream);}
	free(data.recordArray);
  free(data.recordArrayFilterOutput);
  free(data.freqArray);
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
  //1/44100 = 0.00002267573 seconds
	(void) outputBuffer;
	(void) timeInfo;
	(void) statusFlags;
	(void) userData;
 

	int done = paContinue;
	paTestData *data = (paTestData*)userData;
  //SAMPLE *out = (SAMPLE*)outputBuffer; NOT NEEDED JUST YET
  const SAMPLE *readPointer = (const SAMPLE*)inputBuffer; //const
	SAMPLE *writePointerRecordArray    = &data->recordArray[data -> frameIndex * CHANNEL_COUNT]; 
  SAMPLE *writePointerFilterOutArray = &data->recordArrayFilterOutput[data -> frameIndex * CHANNEL_COUNT]; 
  SAMPLE *writePointerFreqArray      = &data->freqArray[data -> freqIndex]; 
  SAMPLE *readPointerHolder;
	long i, framesToCalculate;
	unsigned long remainingFrames = (data -> maxFrameIndex) - (data -> frameIndex);
	
  SAMPLE currentAmp, pastAmp, pastPastAmp;
  SAMPLE peakCount=0.0;
  int ready = 1;
  //PaTime firstPeakTime, secondPeakTime, timeDifference, avgTimeDifference;
 
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
        *writePointerFilterOutArray++ = SAMPLE_SILENCE;
				if(CHANNEL_COUNT==2){
            *writePointerRecordArray++ = SAMPLE_SILENCE; 
            *writePointerFilterOutArray++ = SAMPLE_SILENCE;
            }
		}
     data -> freqEstimate = 0;
	}
	else{
   for(i = 0; i < framesToCalculate; i++){
        readPointerHolder = readPointer++;
				*writePointerRecordArray++  = *readPointerHolder;//was *readPointer++; Problem- the ++ increments
        if(data -> outputSilenceFlag == 1){*writePointerFilterOutArray++ = SAMPLE_SILENCE;}
        else {                             *writePointerFilterOutArray++ = *readPointerHolder;}
       
				if(CHANNEL_COUNT==2){
          readPointerHolder = readPointer++;
          *writePointerRecordArray++ = *readPointerHolder;  
          if(data -> outputSilenceFlag == 1){*writePointerFilterOutArray++ = SAMPLE_SILENCE;}
          else{                              *writePointerFilterOutArray++ = *readPointerHolder;}
          }
          
           currentAmp  = data->recordArray[data->frameIndex*CHANNEL_COUNT+CHANNEL_COUNT*i];
           pastAmp     = data->recordArray[data->frameIndex*CHANNEL_COUNT+CHANNEL_COUNT*i-2];
           pastPastAmp = data->recordArray[data->frameIndex*CHANNEL_COUNT+CHANNEL_COUNT*i-4];
           if(currentAmp < .003){ready = 1;}
           if((pastPastAmp) <= (pastAmp) && (currentAmp) < (pastAmp) && pastAmp > 0 && ready == 1 && pastAmp > .003){   
             ready = 0;   
             peakCount++;
           }
		}
    if( data -> outputSilenceFlag == 1){data -> outputSilenceFlag = 0;}
    if(peakCount > 0){
     data -> freqEstimate = 1/(FRAMES_PER_BUFFER*((1.0/44100.0)/(peakCount)));
     *writePointerFreqArray = data -> freqEstimate;
     if(data -> freqEstimate < 150 || data -> freqEstimate > 1250){
       data -> outputSilenceFlag = 1;
     }
    }
    else{
    *writePointerFreqArray = -1;
    }
	}
	data->frameIndex = data->frameIndex + framesToCalculate;
  data -> freqIndex++;
    return done;
}
