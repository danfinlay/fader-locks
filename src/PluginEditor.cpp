#include "PluginEditor.h"

#include <cmath>

constexpr float LockFadersEditor::kStepPresets[8];

float LockFadersEditor::positionToStep (int pos)
{
    return kStepPresets[juce::jlimit (0, 7, pos)];
}

int LockFadersEditor::stepToNearestPosition (float step)
{
    int   bestIdx  = 0;
    float bestDist = std::abs (step - kStepPresets[0]);
    for (int i = 1; i < 8; ++i)
    {
        const float d = std::abs (step - kStepPresets[i]);
        if (d < bestDist) { bestDist = d; bestIdx = i; }
    }
    return bestIdx;
}

static juce::String formatStepValue (float v)
{
    if (v <= 0.0001f) return "Off  (continuous)";
    const int decimals = (v < 1.0f ? 2 : (v < 10.0f ? 1 : 0));
    return juce::String (v, decimals) + " dB";
}

LockFadersEditor::LockFadersEditor (LockFadersProcessor& p)
    : juce::AudioProcessorEditor (p), processor (p)
{
    setSize (280, 460);

    // ---- Title ----
    titleLabel.setText ("Lock Faders", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions (18.0f, juce::Font::bold)));
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    // ---- Fader ----
    addAndMakeVisible (fader);
    faderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.parameters, "gainDb", fader);

    gainTitleLabel.setText ("Gain", juce::dontSendNotification);
    gainTitleLabel.setJustificationType (juce::Justification::centred);
    gainTitleLabel.setFont (juce::Font (juce::FontOptions (12.0f)));
    addAndMakeVisible (gainTitleLabel);

    // ---- Step knob ----
    stepKnob.setRange (0.0, 7.0, 1.0);
    stepKnob.setRotaryParameters (juce::degreesToRadians (-150.0f),
                                  juce::degreesToRadians ( 150.0f),
                                  true);
    stepKnob.setValue (0.0, juce::dontSendNotification);
    stepKnob.onValueChange = [this]
    {
        if (suppressKnobCallback) return;
        const int   pos = (int) std::round (stepKnob.getValue());
        const float v   = positionToStep (pos);
        writeStepValue (v);
    };
    addAndMakeVisible (stepKnob);

    stepTitleLabel.setText ("Step", juce::dontSendNotification);
    stepTitleLabel.setJustificationType (juce::Justification::centred);
    stepTitleLabel.setFont (juce::Font (juce::FontOptions (12.0f)));
    addAndMakeVisible (stepTitleLabel);

    stepValueLabel.setJustificationType (juce::Justification::centred);
    stepValueLabel.setFont (juce::Font (juce::FontOptions (16.0f, juce::Font::bold)));
    stepValueLabel.setEditable (false, true, false); // double-click to edit
    stepValueLabel.setColour (juce::Label::backgroundWhenEditingColourId,
                              juce::Colours::black.withAlpha (0.4f));
    stepValueLabel.setColour (juce::Label::textWhenEditingColourId, juce::Colours::white);
    stepValueLabel.onTextChange = [this]
    {
        auto txt = stepValueLabel.getText().trim();
        auto cleaned = txt.upToFirstOccurrenceOf ("dB", false, false)
                          .upToFirstOccurrenceOf (" ", false, false)
                          .trim();
        if (cleaned.equalsIgnoreCase ("off") || cleaned.isEmpty())
            writeStepValue (0.0f);
        else
            writeStepValue (juce::jlimit (0.0f, 20.0f, cleaned.getFloatValue()));
    };
    addAndMakeVisible (stepValueLabel);

    stepHintLabel.setText ("double-click to type", juce::dontSendNotification);
    stepHintLabel.setJustificationType (juce::Justification::centred);
    stepHintLabel.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::italic)));
    stepHintLabel.setColour (juce::Label::textColourId,
                             juce::Colours::white.withAlpha (0.5f));
    addAndMakeVisible (stepHintLabel);

    globalIndicatorLabel.setText ("(shared across all instances)",
                                  juce::dontSendNotification);
    globalIndicatorLabel.setJustificationType (juce::Justification::centred);
    globalIndicatorLabel.setFont (juce::Font (juce::FontOptions (9.5f, juce::Font::italic)));
    globalIndicatorLabel.setColour (juce::Label::textColourId,
                                    juce::Colours::white.withAlpha (0.45f));
    addAndMakeVisible (globalIndicatorLabel);

    // Initial sync from current parameter value.
    if (auto* sp = processor.parameters.getRawParameterValue ("step"))
        applyStepToUi (sp->load());

    processor.parameters.addParameterListener ("step", this);
}

LockFadersEditor::~LockFadersEditor()
{
    processor.parameters.removeParameterListener ("step", this);
}

void LockFadersEditor::writeStepValue (float v)
{
    auto* p = processor.parameters.getParameter ("step");
    if (p == nullptr) return;
    p->setValueNotifyingHost (p->convertTo0to1 (v));
}

void LockFadersEditor::applyStepToUi (float v)
{
    fader.setStep ((double) v);

    suppressKnobCallback = true;
    if (v <= 0.0001f)
    {
        stepKnob.setValue (0.0, juce::dontSendNotification);
    }
    else
    {
        // Match an exact preset if possible; otherwise show nearest position
        // without actually changing the underlying step value (it's a custom).
        const int pos = stepToNearestPosition (v);
        stepKnob.setValue ((double) pos, juce::dontSendNotification);
    }
    suppressKnobCallback = false;

    stepValueLabel.setText (formatStepValue (v), juce::dontSendNotification);
}

void LockFadersEditor::parameterChanged (const juce::String& id, float newValue)
{
    if (id != "step") return;
    juce::MessageManager::callAsync ([this, newValue] { applyStepToUi (newValue); });
}

void LockFadersEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient bg (juce::Colour (0xff222428), 0.0f, 0.0f,
                             juce::Colour (0xff15171a), 0.0f, (float) getHeight(),
                             false);
    g.setGradientFill (bg);
    g.fillAll();

    // Subtle separator between fader and knob area.
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.drawVerticalLine (getWidth() / 2, 40.0f, (float) getHeight() - 20.0f);
}

void LockFadersEditor::resized()
{
    auto r = getLocalBounds();
    titleLabel.setBounds (r.removeFromTop (32));

    auto body = r.reduced (8, 4);
    auto left  = body.removeFromLeft (body.getWidth() / 2).reduced (4);
    auto right = body.reduced (4);

    gainTitleLabel.setBounds (left.removeFromTop (18));
    fader.setBounds (left.reduced (6, 0));

    stepTitleLabel.setBounds (right.removeFromTop (18));
    globalIndicatorLabel.setBounds (right.removeFromBottom (14));
    stepHintLabel.setBounds (right.removeFromBottom (16));
    stepValueLabel.setBounds (right.removeFromBottom (28));
    stepKnob.setBounds (right.reduced (4));
}
