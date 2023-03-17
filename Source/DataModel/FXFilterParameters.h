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

    juce::AudioParameterFloat* dryWetMix = nullptr;
    juce::AudioParameterFloat* cutoffFrequency = nullptr;
    juce::AudioParameterChoice* filterType = nullptr;
    juce::AudioParameterChoice* filterSlope = nullptr;

    std::function<void()> update = nullptr;

    void createParameterLayout(juce::AudioProcessorValueTreeState::ParameterLayout &layout)
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> vector;

        dryWetMix = new juce::AudioParameterFloat("filterMix",
                                                  "Wet%",
                                                  juce::NormalisableRange<float>(0.f, 100.f, 0.1), 100.f);
        vector.emplace_back(dryWetMix);
        
        filterType = new juce::AudioParameterChoice("filterType", "Filter Type", typeChoices, 0);
        vector.emplace_back(filterType);

        filterSlope = new juce::AudioParameterChoice("filterSlope", "Filter Slope", slopeChoices, 0);
        vector.emplace_back(filterSlope);

        cutoffFrequency = new juce::AudioParameterFloat("filterCutoff",
                                                        "Cutoff Frequency",
                                                        juce::NormalisableRange<float>(10.f, 22000.f, 0.1, 0.3), 500.f);
        vector.emplace_back(cutoffFrequency);

        layout.add(vector.begin(), vector.end());
    }


    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};