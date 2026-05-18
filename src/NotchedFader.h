#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Vertical dB fader whose mouse-drag motion is constrained to multiples of
// `step` dB *relative to the value at drag-start*. Continuous when step <= 0.
// Host automation and typed text-entry are not constrained.
class NotchedFader : public juce::Slider
{
public:
    NotchedFader();

    void setStep (double newStep) noexcept { step = newStep; }

    void mouseDown      (const juce::MouseEvent&) override;
    void mouseDrag      (const juce::MouseEvent&) override;
    void mouseUp        (const juce::MouseEvent&) override;

    juce::String getTextFromValue (double) override;
    double       getValueFromText (const juce::String&) override;

private:
    double step           { 0.0 };
    double dragStartValue { 0.0 };
    bool   dragging       { false };
};
