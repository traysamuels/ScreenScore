cmake_minimum_required(VERSION 3.22)

# Set architectures before project() so the generated Xcode project actually
# compiles them. Reason 13 on an Intel Mac loads the x86_64 slice; an
# arm64-only binary will not open there. The build script may pass x86_64
# alone when the Mac cannot compile arm64.
if(APPLE)
    if(NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures" FORCE)
    endif()
    if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version" FORCE)
    endif()
endif()

project(VidSyncVST VERSION 1.0.1 LANGUAGES C CXX OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(VidSyncVST
    COMPANY_NAME "VidSync"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    PLUGIN_MANUFACTURER_CODE Vdsy
    PLUGIN_CODE Vds1
    FORMATS VST3
    VST3_CATEGORIES Fx
    PRODUCT_NAME "VidSyncVST"
    DESCRIPTION "Video playback synchronized to the Reason 13 transport"
)

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
    target_sources(VidSyncVST PRIVATE
        Source/MacVideoView.h
        Source/MacVideoView.mm
    )

    target_compile_options(VidSyncVST PRIVATE -fno-objc-arc)

    target_link_libraries(VidSyncVST PRIVATE
        "-framework AVFoundation"
        "-framework AVKit"
        "-framework CoreMedia"
        "-framework Cocoa"
        "-framework QuartzCore"
    )
endif()

target_compile_definitions(VidSyncVST PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
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
