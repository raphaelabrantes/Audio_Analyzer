
#include <iostream>
#include <portaudio.h>

#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_SECONDS     (10)
#define NUM_OF_SAMPLES (NUM_SECONDS * SAMPLE_RATE)
#define NUM_CHANNELS    (2)
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)

typedef struct {
    SAMPLE left;
    SAMPLE right;
} channel;


struct paTestData
{
    int          frameIndex = 0;  /* Index into sample array. */
    int          maxFrameIndex = NUM_OF_SAMPLES;
    std::array<channel, NUM_OF_SAMPLES> recordedSamples{};

};



static unsigned long get_frames_left(unsigned long framesPerBuffer, paTestData *data) {
    auto framesLeft = data->maxFrameIndex - data->frameIndex;
    if( framesLeft < framesPerBuffer )
    {
        data->frameIndex = 0;
        return framesLeft;
    }
    return framesPerBuffer;

}

static int recordCallback( const void *inputBuffer, void *,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo*,
                           PaStreamCallbackFlags ,
                           void *userData){
    auto *data = static_cast<paTestData *>(userData);
    const auto *rptr = static_cast<const SAMPLE *>(inputBuffer);

    auto framesToCalc = get_frames_left(framesPerBuffer, data);

    if(!inputBuffer)
    {
        for(int i=0; i<framesToCalc; i++ )
        {
            auto wptr = data->recordedSamples[data->frameIndex];
            wptr.left = SAMPLE_SILENCE;
            wptr.right =  SAMPLE_SILENCE;  /* right */
            data->recordedSamples[data->frameIndex++] = wptr;
        }
    }
    else
    {
        for(int i=0; i<framesToCalc; i++ ){
            auto wptr = data->recordedSamples[data->frameIndex];
            wptr.left = *rptr++;  /* left */
            wptr.right= *rptr++;  /* right */
            data->recordedSamples[data->frameIndex++] = wptr;
            // std::cout << wptr.left << " " << wptr.right << std::endl;
        }
    }
    return paContinue;
}

void calculate_max_avg(const paTestData &data) {
    double average, val;
    double max = val = average = 0.0;
    ;
    for( int i=0; i< NUM_OF_SAMPLES; i++ )
    {

        val = (std::abs(data.recordedSamples[i].left) + std::abs(data.recordedSamples[i].right)) / 2.0;

        if( val > max )
        {
            max = val;
        }
        average += val;
    }

    average = average / static_cast<double>(NUM_OF_SAMPLES);

    std::cout << "sample max amplitude =" << max << std::endl;
    std::cout << "sample average =" << average << std::endl;

}

/*******************************************************************/
int main()
{
    PaStreamParameters  inputParameters;

    paTestData data;
  /* From now on, recordedSamples is initialised. */


    auto err = Pa_Initialize();
    if( err != paNoError ) goto done;

    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        goto done;
    }
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;

    /* Record some audio. -------------------------------------------- */
    PaStream*           stream;
    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              nullptr,                  /* &outputParameters, */
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              recordCallback,
              &data );
    if( err != paNoError ) goto done;

    err = Pa_StartStream( stream );
    if( err != paNoError ) goto done;
    printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

    while( ( err = Pa_IsStreamActive( stream ) ) == 1 )
    {
        Pa_Sleep(100);
        printf("index = %d\n", data.frameIndex );
        calculate_max_avg(data);


    }
    if( err < 0 ) goto done;

    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto done;

    /* Measure maximum peak amplitude. */


done:
    Pa_Terminate();

    if( err != paNoError )
    {
        fprintf( stderr, "An error occurred while using the portaudio stream\n" );
        fprintf( stderr, "Error number: %d\n", err );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
        err = 1;          /* Always return 0 or 1, but no other return codes. */
    }
    return err;
}