cmake_minimum_required(VERSION 3.22)
project(WaveBombzEndlessSmile VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(WAVEBOMBZ_FETCH_JUCE "Fetch JUCE from upstream when not provided locally" ON)
if(WAVEBOMBZ_FETCH_JUCE)
    FetchContent_Declare(juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.9
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(juce)
else()
    find_package(JUCE CONFIG REQUIRED)
endif()

juce_add_plugin(WaveBombzEndlessSmile
    COMPANY_NAME "WaveBombz"
    BUNDLE_ID "com.wavebombz.endlesssmile"
    PLUGIN_MANUFACTURER_CODE WvBZ
    PLUGIN_CODE WbES
    FORMATS VST3
    PRODUCT_NAME "WaveBombz Endless Smile"
    DESCRIPTION "Intensity-driven multi-effect audio processor"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(WaveBombzEndlessSmile PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_definitions(WaveBombzEndlessSmile PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(WaveBombzEndlessSmile PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags
)
