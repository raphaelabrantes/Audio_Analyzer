
#include <iostream>
#include <portaudio.h>
#include<matplot/matplot.h>
#include <boost/circular_buffer.hpp>

#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_SECONDS     (1)
#define NUM_OF_SAMPLES (NUM_SECONDS * SAMPLE_RATE)
#define NUM_CHANNELS    (2)
#define PA_SAMPLE_TYPE  paInt8
#define SAMPLE_SILENCE  (0)
#define throw_if_error(callback) \
    { PaError err = (callback); \
      if(err != paNoError ) { \
        throw std::runtime_error("Error: " + std::string(Pa_GetErrorText( err ))) ; \
      } \
    };

typedef int8_t SAMPLE;
struct paTestData
{
    boost::circular_buffer <SAMPLE> left_samples {FRAMES_PER_BUFFER};
    boost::circular_buffer <SAMPLE> right_samples {FRAMES_PER_BUFFER};

};




static int recordCallback( const void *inputBuffer, void *,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo*,
                           PaStreamCallbackFlags ,
                           void *userData){
    auto *data = static_cast<paTestData *>(userData);
    const auto *rptr = static_cast<const SAMPLE *>(inputBuffer);


    if(!inputBuffer)
    {
        for(int i=0; i<framesPerBuffer; i++ )
        {
            data->left_samples.push_back(SAMPLE_SILENCE);
            data->right_samples.push_back(SAMPLE_SILENCE);
        }
    }
    else
    {
        for(int i=0; i<framesPerBuffer; i++ ){
            data->left_samples.push_back(*rptr++);
            data->right_samples.push_back(*rptr++);
            // std::cout << wptr.left << " " << wptr.right << std::endl;
        }
    }
    return paContinue;
}


/*******************************************************************/
int main(){

    try {

        auto x = matplot::linspace(0, FRAMES_PER_BUFFER);
        auto f = matplot::figure<>(false);
        f->backend()->run_command("unset warnings");
        auto aux = f->current_axes();
        aux->x_axis().visible(false);
        aux->y_axis().visible(false);
        aux->xlim({0, FRAMES_PER_BUFFER});
        aux->ylim({-128, 127});


        /* From now on, recordedSamples is initialised. */


        throw_if_error(Pa_Initialize());
        PaStreamParameters  inputParameters;
        inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
        inputParameters.channelCount = NUM_CHANNELS;
        inputParameters.sampleFormat = PA_SAMPLE_TYPE;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
        inputParameters.hostApiSpecificStreamInfo = nullptr;

        /* Record some audio. -------------------------------------------- */
        PaStream*           stream;
        paTestData data;
        throw_if_error(Pa_OpenStream(
                  &stream,
                  &inputParameters,
                  nullptr,                  /* &outputParameters, */
                  SAMPLE_RATE,
                  FRAMES_PER_BUFFER,
                  paClipOff,      /* we won't output out of range samples so don't bother clipping them */
                  recordCallback,
                  &data ));

        throw_if_error(Pa_StartStream( stream ));
        std::cout << "\n=== Now recording!! Please speak into the microphone. ===\n" << std::endl;
        while(Pa_IsStreamActive( stream ) == 1)
        {
            Pa_Sleep(100);
            aux->plot(x, data.left_samples , "b", x, data.right_samples, "r" );
        }

        throw_if_error(Pa_CloseStream( stream ));

        /* Measure maximum peak amplitude. */

        throw_if_error(Pa_Terminate());
        return 0;
    } catch (std::runtime_error &e)  {
        std::cerr << "An error occurred while using the portaudio stream" << std::endl;
        std::cerr << "Error number" << e.what();
        return -1;/* Always return 0 or 1, but no other return codes. */
    }
}