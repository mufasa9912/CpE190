#define PA_SAMPLE_TYPE paFloat32
#define SAMPLE_SILENCE 0.0
typedef float SAMPLE;

typedef struct {
    SAMPLE *audioArray; //Using
    SAMPLE *frequencyArray; //Using
    int frameIndex; //Using
    int maxFrameIndex; //Using
    int freqIndex1; //Using
    int freqIndex2; //Using
    int freqVal1; //Using
    int freqVal2; //Using
    int currFreq; //Using
} paTestFreq;

static int paTestCallBack(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData);
