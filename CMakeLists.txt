cmake_minimum_required(VERSION 3.22)

project(Gloubiboulga VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Target architectures")
endif()

# JUCE is expected as a sibling directory: ../JUCE
add_subdirectory(../JUCE JUCE)

# Custom module (fxme_juce_tools)
juce_add_module(../JUCE/usermodules/FxmeJuceTools)

juce_add_plugin(Gloubiboulga
    COMPANY_NAME                "Fx-Mechanics"
    PLUGIN_MANUFACTURER_CODE    FXME
    PLUGIN_CODE                 GLOU
    FORMATS                     VST3 AU
    PRODUCT_NAME                "Gloubiboulga"
    DESCRIPTION                 "Gloubiboulga"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES             "Fx"
    BUNDLE_ID                   "com.FXMechanics.Gloubiboulga"
)

# Embed binary assets (logo)
juce_add_binary_data(GloubiboulgaBinaryData SOURCES
    Source/Assets/logo686.png
)

target_sources(Gloubiboulga PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/BiquadFilter.cpp
    Source/FxmeLogo.cpp
    Source/GlitchVisualizerComponent.cpp
)

# Generate a JuceHeader.h for the target (replaces the Projucer-generated JuceLibraryCode/)
juce_generate_juce_header(Gloubiboulga)

target_compile_definitions(Gloubiboulga
    PUBLIC
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(Gloubiboulga
    PRIVATE
        GloubiboulgaBinaryData
        FxmeJuceTools
        juce_audio_basics
        juce_audio_devices
        juce_audio_formats
        juce_audio_plugin_client
        juce_audio_processors
        juce_audio_processors_headless
        juce_audio_utils
        juce_core
        juce_data_structures
        juce_events
        juce_graphics
        juce_gui_basics
        juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
