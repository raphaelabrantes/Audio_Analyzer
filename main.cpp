
#include <complex>
#include <iostream>
#include <portaudio.h>
#include <GLFW/glfw3.h>
#include <implot.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <boost/circular_buffer.hpp>
#include <vector>

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

struct meansqrt {
    double left, right;
};

struct paTestData
{
    boost::circular_buffer <channels> samples {NUM_OF_SAMPLES};

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

void glfw_error_callback(int error, const char* description) { std::cerr << "GLFW Error " << error << ": " << description << std::endl; }

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
    glfwSetErrorCallback(glfw_error_callback);
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1200, 800, "ImPlot Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Disable vsync

    // Setup context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    // Setup style
    ImGui::StyleColorsDark();

    // Setup backend
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    try {


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
        while(Pa_IsStreamActive( stream ) == 1 && !glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::Begin("Test");
            if (ImPlot::BeginPlot("Test")) {

                ImPlot::SetupAxisLimits(ImAxis_X1, 1300, NUM_OF_SAMPLES);
                ImPlot::SetupAxisLimits(ImAxis_Y1, -128, 127);
                auto left = [](int index, void* ch) {
                    auto sample = static_cast<channels *>(ch)[index];
                    return ImPlotPoint(index, sample.left);
                };

                auto right =  [](int index, void* ch) {
                    auto sample =  static_cast<channels *>(ch)[index];
                    return ImPlotPoint(index, sample.right);
                };
                auto dBFS = calculate_dBFS(&data.samples);
                std::cout << "dBFS: " <<dBFS << std::endl;
                auto samples = data.samples.linearize();

                ImPlot::PlotLineG("Left", left, samples , data.samples.size());
                ImPlot::NextColormapColor();
                ImPlot::PlotLineG("Right", right, samples, data.samples.size());
                ImPlot::EndPlot();
            }
            ImGui::End();
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Swap buffers
            glfwSwapBuffers(window);

        }
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();

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