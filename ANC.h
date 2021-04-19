#define PA_SAMPLE_TYPE paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE (0.0f)
#define PRINT_S_FORMAT "%.8f\t"

typedef struct {
		SAMPLE *recordArray;
    SAMPLE *recordArrayInverse;
		int frameIndex;
		int maxFrameIndex;
} paTestData;

static int paTestCallBack( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData );
