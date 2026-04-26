//
// Created by raphael on 4/13/26.
//

#include "Plotter.h"
void Plotter::update(audio_track *data) const {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Test");
    ImGui::DragFloat("Threshold dBFS",&data->thresh_hold,0.1f,-96,0);
    ImVec4 col = ImPlot::GetLastItemColor();
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
        auto samples = data->samples.linearize();

        ImPlot::PlotLineG("Left", left, samples , data->samples.size());
        ImPlot::NextColormapColor();
        ImPlot::PlotLineG("Right", right, samples, data->samples.size());
        ImPlot::Annotation(0,-128 ,col,ImVec2(0,0),true,"dBFS: %.2f",  data->avg_dBs);

        ImPlot::EndPlot();
    }
    ImGui::End();
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());\
    // Swap buffers
    glfwSwapBuffers(window);

}

bool Plotter::should_close() {
    return glfwWindowShouldClose(window);
}


