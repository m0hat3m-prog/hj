cmake_minimum_required(VERSION 3.22)
project(MaqamPlugin VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")

add_subdirectory(JUCE)

juce_add_plugin(Maqam
    COMPANY_NAME "Maqam Tools"
    BUNDLE_ID com.maqamtools.maqam
    PLUGIN_MANUFACTURER_CODE Mqtl
    PLUGIN_CODE Mqm1
    FORMATS AU VST3
    PRODUCT_NAME "Maqam"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    AU_MAIN_TYPE kAudioUnitType_MIDIProcessor
    VST3_CATEGORIES Fx Tools
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE)

target_sources(Maqam PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(Maqam PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(Maqam
    PRIVATE juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
