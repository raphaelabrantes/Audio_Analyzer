//
// Created by raphael on 4/13/26.
//

#ifndef NOISE_CATCHER_PLOTTER_H
#define NOISE_CATCHER_PLOTTER_H
#include <string>
#include <utility>
#include <GLFW/glfw3.h>
#include <implot.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include "sample.h"


class Plotter {
    public:
    Plotter(int width, int height, std::string title): width(width), height(height), title(std::move(title)) {
        glfwSetErrorCallback(glfw_error_callback);
        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        window = glfwCreateWindow(this->width, this->height, this->title.c_str(), nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window") ;
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

    };

    void update (audio_track *data) const;
    bool should_close();
    ~Plotter() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

private:
    int width;
    int height;
    std::string title;
    GLFWwindow* window;

    static void glfw_error_callback(int error, const char* description) { std::cerr << "GLFW Error " << error << ": " << description << std::endl; }


};

#endif //NOISE_CATCHER_PLOTTER_H