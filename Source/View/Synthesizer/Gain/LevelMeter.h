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

namespace Editor::Synthesizer
{
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
            auto bounds = getLocalBounds().toFloat();
            bounds.reduce(PADDING_PX, PADDING_PX);
            g.setColour(findColour(juce::TextButton::buttonColourId));
            g.fillRoundedRectangle(bounds, 4.f);

            auto scaledValue = juce::jmap(level, LEVEL_METER_LOWER_LIMIT, LEVEL_METER_UPPER_LIMIT, 0.f, bounds.getWidth());
            g.setColour(findColour(juce::TextButton::buttonOnColourId));
            g.fillRoundedRectangle(bounds.removeFromLeft(scaledValue), 4.f);
        }

        void resized() override {}

        void timerCallback() override
        {
            float previousLevel = level;
            level = audioProcessor.atomicSynthRMS[channelIndex].get();

            level = juce::jlimit(LEVEL_METER_LOWER_LIMIT, LEVEL_METER_UPPER_LIMIT, level);
            previousLevel = juce::jlimit(LEVEL_METER_LOWER_LIMIT, LEVEL_METER_UPPER_LIMIT, previousLevel);

            if( level != previousLevel)
                repaint();
        }

    private:
        VST_SynthAudioProcessor& audioProcessor;

        int channelIndex = 0;

        float level = LEVEL_METER_LOWER_LIMIT;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
    };
}