cmake_minimum_required(VERSION 4.0)
project(noise_catcher)

set(CMAKE_CXX_STANDARD 23)
include(FetchContent)
FetchContent_Declare(matplotplusplus
        GIT_REPOSITORY https://github.com/alandefreitas/matplotplusplus
        GIT_TAG origin/master) # or whatever tag you want
FetchContent_GetProperties(matplotplusplus)
if(NOT matplotplusplus_POPULATED)
    FetchContent_MakeAvailable(matplotplusplus)
endif()
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
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIRS} )

add_executable(noise_catcher main.cpp)
target_link_libraries(noise_catcher PUBLIC matplot portaudio m rt asound jack Threads::Threads Boost::headers)