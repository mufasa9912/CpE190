#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "portaudio.h"
#include "pa_linux_alsa.h"
#include "FilterAPI.h"
#define FRAMES_PER_BUFFER 512
#define NUM_OF_CHANNELS 2
#define SAMPLE_RATE 44100
#define LENGTH_OF_SAMPLES 5 //Number of Seconds

static paTestFreq data;
static PaDeviceInfo device;

int main()
{
    printf("Frequency Data Capture Program Initializing");
    paTestFreq data;
    PaStreamParameters inputParameters, outputParameters;
    PaStream *stream;
    PaError err;
    int i, totalFrames, totalSamples, numOfBytes, avgFreq, sampIndex;
    FILE *file;

    err = Pa_Initialize();
    if(err != paNoError) 
        goto error;
    
    //IO Device Declaration
    inputParameters.device = Pa_GetDefaultInputDevice();
    if(inputParameters.device == paNoDevice)
    {
        printf("Input Device Not Found.\n");
        goto error;
    }
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if(outputParameters.device == paNoDevice)
    {
        printf("Output device could not be found.\n");
        goto error;
    }

    //IO Device Setup
    inputParameters.sampleFormat = paFloat32;
    outputParameters.sampleFormat = paFloat32;
    inputParameters.channelCount = NUM_OF_CHANNELS;
    outputParameters.channelCount = NUM_OF_CHANNELS;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowOutputLatency;

    //Data Inialization
    data.frameIndex = 0;
    data.currFreq = 0;
    data.freqIndex1 = 0;
    data.freqIndex2 = 0;
    data.freqVal1 = 0;
    data.freqVal2 = 0;
    data.maxFrameIndex = SAMPLE_RATE * LENGTH_OF_SAMPLES;
    totalFrames = data.maxFrameIndex;
    totalSamples = totalFrames * NUM_OF_CHANNELS;
    numOfBytes = sizeof(SAMPLE) * totalSamples;
    data.audioArray = (SAMPLE *) malloc(numOfBytes);
    if(data.audioArray == NULL)
    {
        printf("\naudioArray could not be initialized");
        free(data.audioArray);
        return -1;
    }
    memset(data.audioArray, 0.0f, totalSamples);
    data.frequencyArray = (SAMPLE *) malloc(numOfBytes);
    if(data.frequencyArray == NULL)
    {
        printf("\nfrequencyArray could not be initialized");
        free(data.frequencyArray);
        return -1;
    }
    memset(data.frequencyArray,0.0f,totalSamples);
    
    //Init
    file = fopen("_audioArrayInit.raw","wb");
    if(file == NULL)
        printf("Could not record to 'audioArrayInit.raw'");
    else
    {
        fwrite(data.audioArray, NUM_OF_CHANNELS * sizeof(SAMPLE),totalFrames, file);
		fclose(file);
		printf("Wrote raw empty audio data to '_audioArrayInit.raw'\n");
    }
    file = fopen("_frequencyArrayInit.csv","wb");
    if(file == NULL)
        printf("Could not record to 'frequencyArrayInit.csv'");
    else
    {
        fwrite(data.frequencyArray, NUM_OF_CHANNELS * sizeof(SAMPLE),totalFrames, file);
		fclose(file);
		printf("Wrote raw empty audio data to 'frequencyArrayInit.csv'\n");
    }
    err = Pa_OpenStream(&stream, &inputParameters, NULL, SAMPLE_RATE,
                        FRAMES_PER_BUFFER, paClipOff, paTestCallBack,
                        &data);
    if(err != paNoError)
        goto error;
    PaAlsa_EnableRealtimeScheduling(stream, 1);
	err = Pa_StartStream(stream);
	if(err != paNoError) 
        goto error;
    while(Pa_IsStreamActive(stream));
    err = Pa_StopStream(stream);
    if(err != paNoError)
        goto error;
    err = Pa_CloseStream(stream);
    if(err != paNoError)
        goto error;

    file = fopen("audioArray.raw","wb");
    if(file == NULL)
        printf("Could not open file\n");
	else 
    {
	    fwrite(data.audioArray, NUM_OF_CHANNELS * sizeof(SAMPLE),totalFrames, file);
		fclose(file);
		printf("Wrote raw audio data to 'audioArray.raw'\n");
    }
    file = fopen("frequencyArray.csv","wb");
    if(file == NULL)
        printf("Could not open file\n");
	else 
    {
	    //fwrite(data.frequencyArray, NUM_OF_CHANNELS * sizeof(SAMPLE),totalFrames, file);
		for(i = 0; i< totalFrames; i++)
        {
            fprintf(file, "SampleNum, Frequencies");
            fprintf(file,"%d,%d\n",data.frameIndex, data.currFreq);
        }
        fclose(file);
		printf("Wrote raw audio data to 'frequencyArray.csv'\n");
    }
    free(data.audioArray);
    free(data.frequencyArray);
    Pa_Terminate();
    if(err != paNoError)
        goto error;

    printf("Ending Stream\n");
    return 0;

    error:
        if(stream)
        { 
            Pa_AbortStream(stream); 
            Pa_CloseStream(stream);
        }
	    free(data.audioArray);
        free(data.frequencyArray);
	    Pa_Terminate();
        fprintf( stderr, "An error occurred while using the portaudio stream\n" );
        fprintf( stderr, "Error number: %d\n", err );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
        return -1;
}

static int paTestCallBack(const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData)
{
    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    int done = paContinue;
    paTestFreq *data = (paTestFreq*) userData;
    const SAMPLE *readPointer = (const SAMPLE*) inputBuffer;
    SAMPLE *writePointerAudioArray = &data->audioArray[data -> frameIndex * NUM_OF_CHANNELS]; 
    SAMPLE *writePointerfrequencyArray = &data->frequencyArray[data -> frameIndex * NUM_OF_CHANNELS]; 
    SAMPLE *readPointerHolder;
    long i, j, framesToCalculate;
	unsigned long remainingFrames = (data -> maxFrameIndex) - (data -> frameIndex);

    if(remainingFrames < framesPerBuffer)
    {
		framesToCalculate = remainingFrames;
		done = paComplete;
	}
	else
    {
		framesToCalculate = framesPerBuffer;
		done = paContinue;
	}

    if(inputBuffer == NULL)
    {
        for(i = 0;i<framesToCalculate; i++)
        {
            for(j = 0;j<NUM_OF_CHANNELS; j++)
            {
                *writePointerAudioArray++ = SAMPLE_SILENCE;
                *writePointerfrequencyArray++ = SAMPLE_SILENCE;
            }
        }
    }
    else
    {
        //Frequency Calculations
        for(i = 0; i < framesToCalculate; i++)
        {
            for(j = 0; j < NUM_OF_CHANNELS; j++)
            {
                readPointerHolder = readPointer++;
                if(i == 0) //Initailize the freq indexes
                {    
                    data->freqVal1 = readPointerHolder;
                    data->freqIndex1 = 0;
                }
                else if(data->freqVal1 > 0 && readPointerHolder < 0) //from pos to neg ISSUE
                {
                    data->freqVal2 = readPointerHolder;
                    data->freqIndex2 = i;
                    data->currFreq = 2*(data->freqIndex2 - data->freqIndex1) * (1/SAMPLE_RATE); //TIME BETWEEN SAMPLES ZEROS APPROX
                }
                else if(data->freqVal2 < 0 && readPointerHolder>0) //from neg to pos ISSUE
                {
                    data->freqVal1 = readPointerHolder; //POSSIBLE ISSUE
                    data->freqIndex1 = i;
                    data->currFreq = 2*(data->freqIndex1 - data->freqIndex2) * (1/SAMPLE_RATE); //TIME BETWEEN SAMPLES ZEROS APPROX
                }
                *writePointerAudioArray++ = *readPointerHolder; // Audio feedthrough
                *writePointerfrequencyArray++ = data->currFreq; //IDK how to do the frequency write back. is it currFreq??
            }
        }
    }
    data -> frameIndex = (data -> frameIndex) + framesToCalculate;
    return done;
}