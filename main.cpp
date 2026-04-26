
#include <chrono>
#include <complex>
#include <iostream>
#include <boost/circular_buffer.hpp>
#include <vector>
#include <mailio/smtp.hpp>
#include <boost/program_options.hpp>

#include "MailSender.h"
#include "Plotter.h"

#define MINUTES 2


std::chrono::time_point begin = std::chrono::steady_clock::now();
bool mailsent = false;

int main(int argc, char* argv[]){
    MailSender mail_sender(argc, argv);

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
        boost::circular_buffer<double> avg_dBFS {MINUTES};
        auto start = std::chrono::steady_clock::now();
        while(Pa_IsStreamActive( stream ) == 1 && !plotter.should_close()) {
            std::chrono::duration<double> elapsed_seconds = std::chrono::steady_clock::now() - start;
            if (elapsed_seconds.count() > 5) {
                avg_dBFS.push_back(calculate_dBFS(&data.samples));
                auto mean = get_avg_dBFS(&avg_dBFS);
                data.avg_dBs = mean;
                std::chrono::duration<double> refresh_mail_timer = begin - std::chrono::steady_clock::now();
                if (refresh_mail_timer.count() > 3600) {
                    begin = std::chrono::steady_clock::now();
                    mailsent = false;
                }
                if (mean > data.thresh_hold && mailsent == false) {
                    mailsent = mail_sender.send_mail(data.thresh_hold, mean);
                }
                start = std::chrono::steady_clock::now();

            }
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
