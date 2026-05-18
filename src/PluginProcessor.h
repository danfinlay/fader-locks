#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "GlobalStep.h"

class FaderLocksProcessor : public juce::AudioProcessor,
                            public juce::AudioProcessorValueTreeState::Listener
{
public:
    FaderLocksProcessor();
    ~FaderLocksProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Fader Locks"; }
    bool   acceptsMidi() const override  { return false; }
    bool   producesMidi() const override { return false; }
    bool   isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int  getNumPrograms() override                              { return 1; }
    int  getCurrentProgram() override                           { return 0; }
    void setCurrentProgram (int) override                       {}
    const juce::String getProgramName (int) override            { return {}; }
    void changeProgramName (int, const juce::String&) override  {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    void parameterChanged (const juce::String& id, float newValue) override;

    juce::AudioProcessorValueTreeState parameters;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    void syncStepParameterFromGlobal (float v);

    juce::SmoothedValue<float> smoothedGain { 1.0f };
    std::atomic<float>*        gainParam { nullptr };
    std::atomic<float>*        stepParam { nullptr };

    GlobalStep::ListenerId globalListenerId { 0 };
    std::atomic<bool>      writingFromGlobal { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaderLocksProcessor)
};
