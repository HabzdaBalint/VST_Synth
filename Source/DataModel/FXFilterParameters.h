/*
==============================================================================

    FXFilterParameters.h
    Created: 16 Mar 2023 2:01:00pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum FilterSlope
{
    s6dBOct,
    s12dBOct,
    s18dBOct,
    s24dBOct
};

enum FilterType
{
    lowPass,
    highPass
};

struct FXFilterParameters : juce::AudioProcessorValueTreeState::Listener
{
    FXFilterParameters(std::function<void()> update) : update(update) {}

    juce::StringArray typeChoices = {"Low-pass", "High-pass"};
    juce::StringArray slopeChoices = {"6dB/Oct", "12dB/Oct", "18dB/Oct", "24dB/Oct"};

    std::function<void()> update = nullptr;

    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> filterGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("filterGroup", "Filter", "|"));

        auto dryWetMix = std::make_unique<juce::AudioParameterFloat>("filterMix",
                                                  "Wet%",
                                                  juce::NormalisableRange<float>(0.f, 100.f, 0.1), 100.f);
        filterGroup.get()->addChild(std::move(dryWetMix));
        
        auto filterType = std::make_unique<juce::AudioParameterChoice>("filterType", "Filter Type", typeChoices, 0);
        filterGroup.get()->addChild(std::move(filterType));

        auto filterSlope = std::make_unique<juce::AudioParameterChoice>("filterSlope", "Filter Slope", slopeChoices, 0);
        filterGroup.get()->addChild(std::move(filterSlope));

        auto cutoffFrequency = std::make_unique<juce::AudioParameterFloat>("filterCutoff",
                                                        "Cutoff Frequency",
                                                        juce::NormalisableRange<float>(10.f, 22000.f, 0.1, 0.3), 500.f);
        filterGroup.get()->addChild(std::move(cutoffFrequency));

        return filterGroup;
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};