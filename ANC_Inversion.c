//Created with the help of different example files provided by PortAudio
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "portaudio.h"
#include "pa_linux_alsa.h"
#include "ANC_Inversion.h"
#define FRAMES_PER_BUFFER (512)
#define CHANNEL_COUNT (2)
#define SAMPLE_RATE (44100)
#define delayUs 22.6757
static paTestData data;
static PaDeviceInfo device;

char chrInput;
double doubleInput = 0.0, ampChange = 1.0;
int totalSamples;
//constant float delayUs = 22.6757; //(1/44100)

int main(){
	printf("ANC PROGRAM STARTING\n\n");
	
	//paTestData data;
	PaStreamParameters inputParameters, outputParameters;
	PaStream *stream;
	PaError err;
	int i, j, totalFrames, numberOfBytes;
	FILE *file;
	SAMPLE avg, max, currentAmp;
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
	
  //Get user input
  printf("\nEnter 'n' for no extra test, 0 to test range of amplitudes, or 1 for a specific change of amplitude.\n");
  scanf("%c", &chrInput);
  if(chrInput == 'n'){
    printf("No extra tests selected.\n");
  }
  else if(chrInput == '0'){
    printf("Range of amplitudes selected.\n");
  }
  else if(chrInput == '1'){
    printf("Enter value for percent amplitude change. Ex: 1 -> 1%% increase\n");
    scanf("%lf", &doubleInput);
    ampChange = ampChange + doubleInput/100.0;
    //printf("ampChange = %lf\n",ampChange);
  }
  else{
    printf("ERROR. Invalid input.\n");
    return -1;
  }
  printf("ampChange = %lf\n",ampChange);
  
	//Initialize data
	data.frameIndex = 0;
	data.maxFrameIndex = SAMPLE_RATE * 5; //5 for seconds to record
	totalFrames = data.maxFrameIndex;
	totalSamples = totalFrames*CHANNEL_COUNT;
	numberOfBytes = sizeof(SAMPLE) * totalSamples;
 
	data.recordArray = (SAMPLE *) malloc( numberOfBytes );
	if(data.recordArray == NULL){ printf("\nrecordArray could not be initialized\n"); free(data.recordArray); return -1;}
	memset(data.recordArray, 0.0f, totalSamples);
  data.recordArrayInverse = (SAMPLE *) malloc( numberOfBytes );
	if(data.recordArrayInverse == NULL){ printf("\nrecordArrayInverse could not be initialized\n"); free(data.recordArrayInverse); return -1;}
	memset(data.recordArrayInverse, 0.0f, totalSamples);
 
  if(chrInput == '0'){
  data.recordArrayInverseAmp1 = (SAMPLE *) malloc( numberOfBytes );
	if(data.recordArrayInverseAmp1 == NULL){ printf("\nrecordArrayInverseAmp1 could not be initialized\n"); free(data.recordArrayInverseAmp1); return -1;}
	memset(data.recordArrayInverseAmp1, 0.0f, totalSamples);
 
  data.recordArrayInverseAmp2 = (SAMPLE *) malloc( numberOfBytes );
	if(data.recordArrayInverseAmp2 == NULL){ printf("\nrecordArrayInverseAmp2 could not be initialized\n"); free(data.recordArrayInverseAmp2); return -1;}
	memset(data.recordArrayInverseAmp2, 0.0f, totalSamples);
 
  data.recordArrayInverseAmp3 = (SAMPLE *) malloc( numberOfBytes );
	if(data.recordArrayInverseAmp3 == NULL){ printf("\nrecordArrayInverseAmp3 could not be initialized\n"); free(data.recordArrayInverseAmp3); return -1;}
	memset(data.recordArrayInverseAmp3, 0.0f, totalSamples);
 
  data.recordArrayInverseAmpNeg1 = (SAMPLE *) malloc( numberOfBytes );
	if(data.recordArrayInverseAmpNeg1 == NULL){ printf("\nrecordArrayInverseAmpNeg1 could not be initialized\n"); free(data.recordArrayInverseAmpNeg1); return -1;}
	memset(data.recordArrayInverseAmpNeg1, 0.0f, totalSamples);
 
  data.recordArrayInverseAmpNeg2 = (SAMPLE *) malloc( numberOfBytes );
	if(data.recordArrayInverseAmpNeg2 == NULL){ printf("\nrecordArrayInverseAmpNeg2 could not be initialized\n"); free(data.recordArrayInverseAmpNeg2); return -1;}
	memset(data.recordArrayInverseAmpNeg2, 0.0f, totalSamples);
 
  data.recordArrayInverseAmpNeg3 = (SAMPLE *) malloc( numberOfBytes );
	if(data.recordArrayInverseAmpNeg3 == NULL){ printf("\nrecordArrayInverseAmpNeg3 could not be initialized\n"); free(data.recordArrayInverseAmpNeg3);  return -1;}
	memset(data.recordArrayInverseAmpNeg3, 0.0f, totalSamples);
  }

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
                                SAMPLE_RATE,/* SAMPLE_RATE */
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
	
  while(Pa_IsStreamActive(stream));
	
	err = Pa_StopStream(stream); //Pa_AbortStream() is an option too
	if(err != paNoError) goto error;
	err = Pa_CloseStream(stream);
	if(err != paNoError) goto error;
	
 printf("\n");
 
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
 
 printf("\n\nA delay of one sample leads to a 22.6757 microsecond delay.\n");
 for(i = 0; i < totalSamples; i++){
       currentAmp = data.recordArray[i];
       if(currentAmp < 0){currentAmp = currentAmp * -1.0;}
       if(currentAmp > max){max = currentAmp;}
       avg = avg + currentAmp;
   }
   avg = avg/totalSamples;
   printf("Analysis of the raw input:\n");
   printf("Avg Amplitude: %.10f, Max Amplitude: %.10f\n\n", avg, max);
   
 if(chrInput == 'n' || chrInput == '1'){
     printf("======================================================================\n");
     audioAnalysis(0, ampChange); //recordArrayInverse
     printf("======================================================================\n");
 }
 else if(chrInput == '0'){
     printf("======================================================================\n");
     audioAnalysis(0, 1); //recordArrayInverse
     printf("======================================================================\n");
     audioAnalysis(1,-1); //recordArrayInverseAmp1
     printf("======================================================================\n");
     audioAnalysis(2,-1); //recordArrayInverseAmp2
     printf("======================================================================\n");
     audioAnalysis(3,-1); //recordArrayInverseAmp3
     printf("======================================================================\n");
     audioAnalysis(4,-1); //recordArrayInverseAmpNeg1
     printf("======================================================================\n");
     audioAnalysis(5,-1); //recordArrayInverseAmpNeg2
     printf("======================================================================\n");
     audioAnalysis(6,-1); //recordArrayInverseAmpNeg3
     printf("======================================================================\n");
 }
 
	free(data.recordArray);
  free(data.recordArrayInverse);
  if(chrInput == '0'){
    free(data.recordArrayInverseAmp1);
    free(data.recordArrayInverseAmp2);
    free(data.recordArrayInverseAmp3);
    free(data.recordArrayInverseAmpNeg1);
    free(data.recordArrayInverseAmpNeg2);
    free(data.recordArrayInverseAmpNeg3);
  }
	err = Pa_Terminate();
	if(err != paNoError) goto error;
	
	printf("ENDING AUDIO STREAM\n");
	return 0;
	
error: //DEFAULT ERROR DISPLAY CODE
	if(stream){ Pa_AbortStream(stream); Pa_CloseStream(stream);}
	free(data.recordArray);
  free(data.recordArrayInverse);
  if(chrInput == '0'){
    free(data.recordArrayInverseAmp1);
    free(data.recordArrayInverseAmp2);
    free(data.recordArrayInverseAmp3);
    free(data.recordArrayInverseAmpNeg1);
    free(data.recordArrayInverseAmpNeg2);
    free(data.recordArrayInverseAmpNeg3);
  }
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
   if(chrInput == 'n' || chrInput == '1'){
   for(i = 0; i < framesToCalculate; i++){
        readPointerHolder = readPointer++;//LEFT C
				*writePointerRecordArray++  = *readPointerHolder;        //was *readPointer++; Problem- the ++ increments
        *writePointerInverseArray++ = (-1.0)*(*readPointerHolder)*ampChange;   // * percentIncrease
				if(CHANNEL_COUNT==2){
          readPointerHolder = readPointer++;//RIGHT C
          *writePointerRecordArray++  = *readPointerHolder;  
          *writePointerInverseArray++ = (-1.0)*(*readPointerHolder)*ampChange;  // * percentIncrease
          }
		}
   }
   else if(chrInput == '0'){
   SAMPLE *writePointerInverseArrayAmp1    = &data->recordArrayInverseAmp1[data -> frameIndex * CHANNEL_COUNT]; 
   SAMPLE *writePointerInverseArrayAmp2    = &data->recordArrayInverseAmp2[data -> frameIndex * CHANNEL_COUNT]; 
   SAMPLE *writePointerInverseArrayAmp3    = &data->recordArrayInverseAmp3[data -> frameIndex * CHANNEL_COUNT]; 
   SAMPLE *writePointerInverseArrayNegAmp1 = &data->recordArrayInverseAmpNeg1[data -> frameIndex * CHANNEL_COUNT]; 
   SAMPLE *writePointerInverseArrayNegAmp2 = &data->recordArrayInverseAmpNeg2[data -> frameIndex * CHANNEL_COUNT]; 
   SAMPLE *writePointerInverseArrayNegAmp3 = &data->recordArrayInverseAmpNeg3[data -> frameIndex * CHANNEL_COUNT]; 
   for(i = 0; i < framesToCalculate; i++){
        readPointerHolder = readPointer++;
				*writePointerRecordArray++  = *readPointerHolder;        //was *readPointer++; Problem- the ++ increments
        *writePointerInverseArray++ = (-1.0)*(*readPointerHolder);
        *writePointerInverseArrayAmp1++ = (-1.0)*(*readPointerHolder)*(1.01);
        *writePointerInverseArrayAmp2++ = (-1.0)*(*readPointerHolder)*(1.02);
        *writePointerInverseArrayAmp3++ = (-1.0)*(*readPointerHolder)*(1.03);
        *writePointerInverseArrayNegAmp1++ = (-1.0)*(*readPointerHolder)*(.99);
        *writePointerInverseArrayNegAmp2++ = (-1.0)*(*readPointerHolder)*(.98);
        *writePointerInverseArrayNegAmp3++ = (-1.0)*(*readPointerHolder)*(.97);
				if(CHANNEL_COUNT==2){
          readPointerHolder = readPointer++;
          *writePointerRecordArray++  = *readPointerHolder;  
          *writePointerInverseArray++ = (-1.0)*(*readPointerHolder);
          *writePointerInverseArrayAmp1++ = (-1.0)*(*readPointerHolder)*(1.01);
          *writePointerInverseArrayAmp2++ = (-1.0)*(*readPointerHolder)*(1.02);
          *writePointerInverseArrayAmp3++ = (-1.0)*(*readPointerHolder)*(1.03);
          *writePointerInverseArrayNegAmp1++ = (-1.0)*(*readPointerHolder)*(.99);
          *writePointerInverseArrayNegAmp2++ = (-1.0)*(*readPointerHolder)*(.98);
          *writePointerInverseArrayNegAmp3++ = (-1.0)*(*readPointerHolder)*(.97);
          }
		}
   }
	}
   	data->frameIndex = (data->frameIndex) + framesToCalculate;
    return done;
}

void audioAnalysis(int num, double ampChange){

if(num == 0){printf("Analysis with an amplitude change of %.2lf%%\n\n", 100*(ampChange-1.0));}
else if(num == 1){printf("Analysis with an amplitude change of +1%%\n\n");}
else if(num == 2){printf("Analysis with an amplitude change of +2%%\n\n");}
else if(num == 3){printf("Analysis with an amplitude change of +3%%\n\n");}
else if(num == 4){printf("Analysis with an amplitude change of -1%%\n\n");}
else if(num == 5){printf("Analysis with an amplitude change of -2%%\n\n");}
else if(num == 6){printf("Analysis with an amplitude change of -3%%\n\n");}
else{printf("ERROR 0 IN AUDIO ANALYSIS");}


int i, j;
 for(i = 0; i < 16; i++){
   SAMPLE max = 0, avg = 0, currentAmp = 0;
   if(i == 1) printf("Analysis with an output delay of %d sample  (%3.4f microseconds):\n", i, delayUs*i);
   else  printf("Analysis with an output delay of %d samples (%3.4f microseconds):\n", i, delayUs*i);
   for(j = 0; j < totalSamples; j++){ //j-i might be wrong
       if(num == 0){currentAmp = data.recordArray[j] + data.recordArrayInverse[j-i];}
       else if(num == 1){currentAmp = data.recordArray[j] + data.recordArrayInverseAmp1[j-i];}
       else if(num == 2){currentAmp = data.recordArray[j] + data.recordArrayInverseAmp2[j-i];}
       else if(num == 3){currentAmp = data.recordArray[j] + data.recordArrayInverseAmp3[j-i];}
       else if(num == 4){currentAmp = data.recordArray[j] + data.recordArrayInverseAmpNeg1[j-i];}
       else if(num == 5){currentAmp = data.recordArray[j] + data.recordArrayInverseAmpNeg2[j-i];}
       else if(num == 6){currentAmp = data.recordArray[j] + data.recordArrayInverseAmpNeg3[j-i];}
       else{printf("ERROR 1 IN AUDIO ANALYSIS");}
       if(currentAmp < 0){currentAmp = currentAmp * -1.0;}
       if(currentAmp > max){max = currentAmp;}
       avg = avg + currentAmp;
   }
   avg = avg/totalSamples;
   printf("Avg Amplitude: %.10f, Max Amplitude: %.10f\n\n", avg, max);
 }
}
