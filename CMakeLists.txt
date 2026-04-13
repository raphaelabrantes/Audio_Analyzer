cmake_minimum_required(VERSION 4.0)
list(APPEND VCPKG_MANIFEST_FEATURES glfw-binding opengl3-binding)
project(noise_catcher)

set(CMAKE_CXX_STANDARD 23)
include(FetchContent)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

FetchContent_Declare(portaudio
        GIT_REPOSITORY https://github.com/PortAudio/portaudio
        GIT_TAG origin/master) # or whatever tag you want
FetchContent_GetProperties(portaudio)
if(NOT portaudio_POPULATED)
    FetchContent_MakeAvailable(portaudio)
endif()
find_package(Boost REQUIRED)
find_package(OpenGL REQUIRED)
find_package(implot REQUIRED)
find_package(imgui REQUIRED)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIRS} )

add_executable(noise_catcher main.cpp
        Plotter.cpp
        Plotter.h
        sample.h
)
target_link_libraries(noise_catcher PUBLIC portaudio m rt asound jack Threads::Threads Boost::headers glfw OpenGL::GL implot::implot imgui::imgui )