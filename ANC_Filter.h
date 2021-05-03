#define PA_SAMPLE_TYPE paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE (0.0f)
#define PRINT_S_FORMAT "%.10f\t"

typedef struct {
		SAMPLE *recordArray;
    SAMPLE *recordArrayFilterOutput;
    SAMPLE *freqArray;
    SAMPLE freqEstimate;
    int outputSilenceFlag;
		int frameIndex;
		int maxFrameIndex;
    int freqIndex;
} paTestData;

static int paTestCallBack( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData );