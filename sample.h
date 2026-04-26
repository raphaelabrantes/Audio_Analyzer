//
// Created by raphael on 4/13/26.
//

#ifndef NOISE_CATCHER_SAMPLE_H
#define NOISE_CATCHER_SAMPLE_H
#include <cmath>
#include <portaudio.h>
#include <boost/circular_buffer.hpp>
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_SECONDS     (10)
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
    float thresh_hold = -20.0;
    double avg_dBs = 0.0;
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
struct meansqrt {
    double left, right;
};

inline meansqrt calculate_mean_sqrt(boost::circular_buffer<channels> *samples) {
    double value_left = 0;
    double value_right = 0;

    for (auto sample: *samples) {

        value_left += std::pow(sample.left, 2) + 1;
        value_right += std::pow(sample.right, 2) + 1;
    }
    double n = samples->size();
    value_left *= 1.0/ n;
    value_right *= 1.0/ n;
    return {std::sqrt(value_left), std::sqrt(value_right)};
}

inline double calculate_dBFS(boost::circular_buffer<channels> * samples) {

    auto meansqrt = calculate_mean_sqrt(samples);
    const auto value_left = std::log10(meansqrt.left / 127.0) * 20;
    const auto value_right = std::log10(meansqrt.right / 127.0) * 20;
    const double intensity_left = 1e-12 * std::pow(10, value_left/10.0);
    const double intensity_right = 1e-12 * std::pow(10, value_right/10.0);
    const double intensity = intensity_right + intensity_left;
    return 10 * std::log10(intensity / 1e-12);

};

inline double get_avg_dBFS(boost::circular_buffer<double> *avg_dBFS) {
    double a = 0;
    for (const auto avg_d_bfs: *avg_dBFS) {
        a += avg_d_bfs;
    }
    return a / static_cast<double>(avg_dBFS->size());
}

#endif //NOISE_CATCHER_SAMPLE_H