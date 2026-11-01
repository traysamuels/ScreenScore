cmake_minimum_required(VERSION 3.22)
project(VidSyncVST VERSION 1.0.0 LANGUAGES C CXX OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Target Universal Binary for macOS (Apple Silicon arm64 + Intel x86_64)
if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build universal binary" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X version" FORCE)
endif()

# Fetch JUCE automatically via CMake FetchContent
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4 # or 7.0.12
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# Configure Plugin target
juce_add_plugin(VidSyncVST
    COMPANY_NAME "VidSync"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    PLUGIN_MANUFACTURER_CODE Vdsy
    PLUGIN_CODE Vds1
    FORMATS VST3 AU
    PRODUCT_NAME "VidSyncVST"
    DESCRIPTION "Hardware-accelerated DAW Video Sync Plugin for Reason 13 & macOS"
)

# Source files
target_sources(VidSyncVST
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/VideoPlayerComponent.cpp
        Source/VideoPlayerComponent.h
        Source/TimecodeOverlay.h
        Source/DetachedVideoWindow.h
)

if (APPLE)
    target_sources(VidSyncVST
        PRIVATE
            Source/MacVideoView.h
            Source/MacVideoView.mm
    )
    # Objective-C++ ARC and macOS Frameworks
    target_compile_options(VidSyncVST PRIVATE -fobjc-arc)
    target_link_libraries(VidSyncVST
        PRIVATE
            "-framework AVFoundation"
            "-framework AVKit"
            "-framework CoreMedia"
            "-framework Cocoa"
            "-framework QuartzCore"
    )
endif()

target_compile_definitions(VidSyncVST
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(VidSyncVST
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_extra
        juce::juce_gui_basics
        juce::juce_graphics
        juce::juce_events
        juce::juce_core
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
