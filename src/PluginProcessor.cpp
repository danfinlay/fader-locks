#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr float kMinDb = -60.0f;
    constexpr float kMaxDb =  12.0f;
}

juce::AudioProcessorValueTreeState::ParameterLayout
LockFadersProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "gainDb", 1 },
        "Gain",
        juce::NormalisableRange<float> (kMinDb, kMaxDb, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel ("dB")
            .withStringFromValueFunction ([] (float v, int)
            {
                if (v <= kMinDb + 0.001f) return juce::String ("-inf");
                return juce::String (v, 2);
            })));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "step", 1 },
        "Step",
        juce::NormalisableRange<float> (0.0f, 20.0f, 0.0f),
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel ("dB")
            .withStringFromValueFunction ([] (float v, int)
            {
                if (v <= 0.0001f) return juce::String ("Off");
                return juce::String (v, (v < 1.0f ? 2 : (v < 10.0f ? 1 : 0))) + " dB";
            })));

    return layout;
}

LockFadersProcessor::LockFadersProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createLayout())
{
    gainParam = parameters.getRawParameterValue ("gainDb");
    stepParam = parameters.getRawParameterValue ("step");

    parameters.addParameterListener ("step",   this);
    parameters.addParameterListener ("gainDb", this);

    // Adopt whatever the process-wide step already is, if any other instance
    // set it before us.
    syncStepParameterFromGlobal (GlobalStep::get().getStep());

    globalListenerId = GlobalStep::get().addListener (
        [this] (float v)
        {
            juce::MessageManager::callAsync ([this, v]
            {
                syncStepParameterFromGlobal (v);
            });
        });
}

LockFadersProcessor::~LockFadersProcessor()
{
    GlobalStep::get().removeListener (globalListenerId);
    parameters.removeParameterListener ("step",   this);
    parameters.removeParameterListener ("gainDb", this);
}

void LockFadersProcessor::syncStepParameterFromGlobal (float v)
{
    auto* p = parameters.getParameter ("step");
    if (p == nullptr) return;

    const float target = p->convertTo0to1 (juce::jlimit (0.0f, 20.0f, v));
    if (std::abs (p->getValue() - target) < 1.0e-6f) return;

    writingFromGlobal.store (true, std::memory_order_release);
    p->setValueNotifyingHost (target);
    writingFromGlobal.store (false, std::memory_order_release);
}

void LockFadersProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    smoothedGain.reset (sampleRate, 0.02);
    smoothedGain.setCurrentAndTargetValue (
        juce::Decibels::decibelsToGain (gainParam->load(), kMinDb));
}

bool LockFadersProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    if (in.isDisabled() || out.isDisabled()) return false;
    return in == out && in.size() >= 1;
}

void LockFadersProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& /*midi*/)
{
    juce::ScopedNoDenormals noDenormals;

    const float db        = gainParam->load();
    const float targetLin = juce::Decibels::decibelsToGain (db, kMinDb);
    smoothedGain.setTargetValue (targetLin);

    const int numCh      = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int s = 0; s < numSamples; ++s)
    {
        const float g = smoothedGain.getNextValue();
        for (int ch = 0; ch < numCh; ++ch)
            buffer.getWritePointer (ch)[s] *= g;
    }
}

juce::AudioProcessorEditor* LockFadersProcessor::createEditor()
{
    return new LockFadersEditor (*this);
}

void LockFadersProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void LockFadersProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
    {
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
        // Make the loaded step value the new process-wide value.
        GlobalStep::get().setStep (stepParam->load());
    }
}

void LockFadersProcessor::parameterChanged (const juce::String& id, float newValue)
{
    if (id == "step")
    {
        if (writingFromGlobal.load (std::memory_order_acquire))
            return;
        GlobalStep::get().setStep (newValue);
    }
}

// JUCE entry point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LockFadersProcessor();
}
