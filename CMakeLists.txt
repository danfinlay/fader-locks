cmake_minimum_required(VERSION 3.22)
project(FaderLocks VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(FaderLocks
    COMPANY_NAME "FaderLocks"
    BUNDLE_ID "com.faderlocks.faderlocks"
    PLUGIN_MANUFACTURER_CODE Flok
    PLUGIN_CODE Fdlk
    FORMATS VST3
    PRODUCT_NAME "Fader Locks"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES "Fx" "Tools"
)

target_sources(FaderLocks PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/GlobalStep.cpp
    src/NotchedFader.cpp
)

target_compile_definitions(FaderLocks PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(FaderLocks
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
