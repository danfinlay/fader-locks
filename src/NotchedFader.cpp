#include "NotchedFader.h"

#include <cmath>

NotchedFader::NotchedFader()
    : juce::Slider (juce::Slider::SliderStyle::LinearVertical,
                    juce::Slider::TextEntryBoxPosition::TextBoxBelow)
{
    setRange (-60.0, 12.0, 0.0);
    setValue (0.0, juce::dontSendNotification);
    setDoubleClickReturnValue (true, 0.0);
    setSliderSnapsToMousePosition (false);
    setVelocityBasedMode (false);
    setMouseDragSensitivity (300);
    setTextBoxStyle (juce::Slider::TextEntryBoxPosition::TextBoxBelow,
                     false, 64, 20);
}

void NotchedFader::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isLeftButtonDown() && ! e.mods.isPopupMenu())
    {
        dragStartValue = getValue();
        dragging       = true;
    }
    juce::Slider::mouseDown (e);
}

void NotchedFader::mouseDrag (const juce::MouseEvent& e)
{
    if (! dragging)
    {
        juce::Slider::mouseDrag (e);
        return;
    }

    const double range = getMaximum() - getMinimum();
    if (range <= 0.0)
        return;

    const int    sensitivity   = getMouseDragSensitivity();
    const double valuePerPixel = range / (double) juce::jmax (1, sensitivity);

    // Drag distance in pixels from where the drag began; up = increase.
    const double dy        = (double) e.getDistanceFromDragStartY();
    const double rawDelta  = -dy * valuePerPixel;
    const double appliedDelta = (step <= 0.0)
                                    ? rawDelta
                                    : std::round (rawDelta / step) * step;

    const double newValue = juce::jlimit (getMinimum(), getMaximum(),
                                          dragStartValue + appliedDelta);

    if (newValue != getValue())
        setValue (newValue, juce::sendNotificationSync);
}

void NotchedFader::mouseUp (const juce::MouseEvent& e)
{
    dragging = false;
    juce::Slider::mouseUp (e);
}

juce::String NotchedFader::getTextFromValue (double v)
{
    if (v <= getMinimum() + 0.0001)
        return "-inf dB";
    return juce::String (v, 2) + " dB";
}

double NotchedFader::getValueFromText (const juce::String& text)
{
    auto cleaned = text.upToFirstOccurrenceOf ("dB", false, false).trim();
    if (cleaned.equalsIgnoreCase ("-inf") || cleaned.equalsIgnoreCase ("-infinity"))
        return getMinimum();
    return cleaned.getDoubleValue();
}
