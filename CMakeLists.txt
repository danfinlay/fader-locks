cmake_minimum_required(VERSION 3.22)
project(LockFaders VERSION 0.1.0 LANGUAGES C CXX)

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

juce_add_plugin(LockFaders
    COMPANY_NAME "LockFaders"
    BUNDLE_ID "com.lockfaders.lockfaders"
    PLUGIN_MANUFACTURER_CODE Lkfd
    PLUGIN_CODE Lkfr
    FORMATS VST3
    PRODUCT_NAME "Lock Faders"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES "Fx" "Tools"
)

target_sources(LockFaders PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/GlobalStep.cpp
    src/NotchedFader.cpp
)

target_compile_definitions(LockFaders PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(LockFaders
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
