cmake_minimum_required(VERSION 3.22)
project(FretIQ VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Tell CMake where JUCE is installed
set(JUCE_DIR "/Users/alikhatibak/JUCE/install")
find_package(JUCE CONFIG REQUIRED PATHS ${JUCE_DIR})

# Define a JUCE GUI application **before linking libraries**
juce_add_gui_app(FretIQ
    PRODUCT_NAME "FretIQ"
)

# Ensure JuceHeader.h gets generated
juce_generate_juce_header(FretIQ)

# Add source files
target_sources(FretIQ PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
)

# Include JUCE headers
target_include_directories(FretIQ PRIVATE ${JUCE_DIR}/include/JUCE-8.0.6/modules)

# --- AUBIO CONFIGURATION ---
set(AUBIO_ROOT /opt/homebrew/opt/aubio)
set(AUBIO_INCLUDE_DIR ${AUBIO_ROOT}/include)
set(AUBIO_LIB_DIR ${AUBIO_ROOT}/lib)

# Ensure CMake knows where to find Aubio
target_include_directories(FretIQ PRIVATE ${AUBIO_INCLUDE_DIR})
target_link_directories(FretIQ PRIVATE ${AUBIO_LIB_DIR})

# Link JUCE modules
target_link_libraries(FretIQ PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_utils
    juce::juce_gui_basics
    juce::juce_events
    juce::juce_graphics
    juce::juce_data_structures
    juce::juce_core
    juce::juce_audio_devices
)

# Link Aubio after defining FretIQ
target_link_libraries(FretIQ PRIVATE aubio)

