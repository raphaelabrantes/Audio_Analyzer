
#include <complex>
#include <iostream>
#include <boost/circular_buffer.hpp>
#include <vector>
#include "Plotter.h"


struct meansqrt {
    double left, right;
};



meansqrt calculate_mean_sqrt(boost::circular_buffer<channels> *samples) {
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

double calculate_dBFS(boost::circular_buffer<channels> * samples) {

    auto meansqrt = calculate_mean_sqrt(samples);
    const auto value_left = std::log10(meansqrt.left / 127.0) * 20;
    const auto value_right = std::log10(meansqrt.right / 127.0) * 20;
    const double intensity_left = 1e-12 * std::pow(10, value_left/10.0);
    const double intensity_right = 1e-12 * std::pow(10, value_right/10.0);
    const double intensity = intensity_right + intensity_left;
    return 10 * std::log10(intensity / 1e-12);

};

/*******************************************************************/
int main(){

    try {
        Plotter plotter(1200, 800, "ImPlot Example");

        throw_if_error(Pa_Initialize());
        PaStreamParameters  inputParameters;
        inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
        inputParameters.channelCount = NUM_CHANNELS;
        inputParameters.sampleFormat = PA_SAMPLE_TYPE;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
        inputParameters.hostApiSpecificStreamInfo = nullptr;

        /* Record some audio. -------------------------------------------- */
        PaStream*           stream;
        audio_track data;
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
        while(Pa_IsStreamActive( stream ) == 1 && !plotter.should_close()) {
            plotter.update(&data);
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