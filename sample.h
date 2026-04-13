//
// Created by raphael on 4/13/26.
//

#ifndef NOISE_CATCHER_SAMPLE_H
#define NOISE_CATCHER_SAMPLE_H
#include <portaudio.h>
#include <boost/circular_buffer.hpp>
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_SECONDS     (5)
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

struct channels {
    SAMPLE left;
    SAMPLE right;
};


struct audio_track
{
    boost::circular_buffer <channels> samples {NUM_OF_SAMPLES};

};


static int recordCallback( const void *inputBuffer, void *,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo*,
                           PaStreamCallbackFlags ,
                           void *userData){
    auto *data = static_cast<audio_track *>(userData);
    const auto *rptr = static_cast<const SAMPLE *>(inputBuffer);


    if(!inputBuffer)
    {
        for(int i=0; i<framesPerBuffer; i++ )
        {
            channels ch = {
                SAMPLE_SILENCE,
                SAMPLE_SILENCE
            };
            data->samples.push_back(ch);
        }
    }
    else
    {
        for(int i=0; i<framesPerBuffer; i++ ){
            channels ch = {
                *rptr++,
                *rptr++
            };
            data->samples.push_back(ch);
            // std::cout << wptr.left << " " << wptr.right << std::endl;
        }
    }
    return paContinue;
}





#endif //NOISE_CATCHER_SAMPLE_H