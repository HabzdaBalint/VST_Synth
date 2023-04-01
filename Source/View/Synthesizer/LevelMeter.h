/*
==============================================================================

    LevelMeter.h
    Created: 31 Mar 2023 8:21:28pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

class LevelMeter : public juce::Component,
                   public juce::Timer
{
public:
    LevelMeter(VST_SynthAudioProcessor& p, int channelIndex) : audioProcessor(p), channelIndex(channelIndex)
    {
        startTimerHz(60);
    }

    ~LevelMeter() override {}

    void paint(juce::Graphics& g) override
    {
        level = audioProcessor.atomicSynthRMS[channelIndex].get();

        if(level < -90.f)
            level = -90.f;

        auto bounds = getLocalBounds().toFloat();
        bounds.reduce(PADDING_PX, PADDING_PX);
        g.setColour(findColour(juce::GroupComponent::outlineColourId));
        g.fillRoundedRectangle(bounds, 4.f);

        auto scaledValue = juce::jmap(level, -90.f, 6.f, 0.f, bounds.getWidth());
        g.setColour(findColour(juce::Slider::textBoxTextColourId));
        g.fillRoundedRectangle(bounds.removeFromLeft(scaledValue), 4.f);
    }

    void resized() override
    {
        repaint();
    }

    void timerCallback() override
    {
        repaint();
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    int channelIndex = 0;

    float level = -90.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};
