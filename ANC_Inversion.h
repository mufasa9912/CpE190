#define PA_SAMPLE_TYPE paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE (0.0f)
#define PRINT_S_FORMAT "%.10f\t"

typedef struct {
    SAMPLE *recordArray;
    SAMPLE *recordArrayInverse;
    SAMPLE *recordArrayInverseAmp1;
    SAMPLE *recordArrayInverseAmp2;
    SAMPLE *recordArrayInverseAmp3;
    SAMPLE *recordArrayInverseAmpNeg1;
    SAMPLE *recordArrayInverseAmpNeg2;
    SAMPLE *recordArrayInverseAmpNeg3;
    int frameIndex;
    int maxFrameIndex;
} paTestData;

static int paTestCallBack(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

void audioAnalysis(int num, double ampChange);