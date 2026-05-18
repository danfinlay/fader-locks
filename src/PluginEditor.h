#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "NotchedFader.h"
#include "PluginProcessor.h"

class LockFadersEditor : public juce::AudioProcessorEditor,
                         public juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit LockFadersEditor (LockFadersProcessor&);
    ~LockFadersEditor() override;

    void paint   (juce::Graphics&) override;
    void resized() override;

    void parameterChanged (const juce::String& id, float newValue) override;

    // Preset detent values for the step knob (in dB). Index 0 is "Off"
    // (continuous), then descending step sizes.
    static constexpr float kStepPresets[8] = {
        0.0f, 10.0f, 5.0f, 3.0f, 2.0f, 1.0f, 0.5f, 0.25f
    };

private:
    static float positionToStep (int pos);
    static int   stepToNearestPosition (float step);

    void writeStepValue (float v);
    void applyStepToUi  (float v);

    LockFadersProcessor& processor;

    juce::Label   titleLabel;
    NotchedFader  fader;
    juce::Label   gainTitleLabel;

    juce::Slider  stepKnob { juce::Slider::SliderStyle::RotaryVerticalDrag,
                             juce::Slider::TextEntryBoxPosition::NoTextBox };
    juce::Label   stepTitleLabel;
    juce::Label   stepValueLabel;
    juce::Label   stepHintLabel;
    juce::Label   globalIndicatorLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> faderAttachment;

    bool suppressKnobCallback { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LockFadersEditor)
};
