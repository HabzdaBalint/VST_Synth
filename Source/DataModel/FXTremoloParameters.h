/*
==============================================================================

    FXTremoloParameters.h
    Created: 19 Mar 2023 7:19:03pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct FXTremoloParameters : public juce::AudioProcessorValueTreeState::Listener
{
    FXTremoloParameters(std::function<void()> update) : update(update) {}

    juce::AudioParameterFloat* mix = nullptr;
    juce::AudioParameterFloat* depth = nullptr;
    juce::AudioParameterFloat* rate = nullptr;
    juce::AudioParameterBool* isAutoPan = nullptr;

    std::function<void()> update = nullptr;

    void createParameterLayout(juce::AudioProcessorValueTreeState::ParameterLayout &layout)
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> vector;

        mix = new juce::AudioParameterFloat("tremoloMix", 
                                            "Wet%",
                                            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 100.f);
        vector.emplace_back(mix);

        depth = new juce::AudioParameterFloat("tremoloDepth", 
                                              "Depth",
                                              juce::NormalisableRange<float>(0.f, 100.f, 0.1), 50.f);
        vector.emplace_back(depth);

        rate = new juce::AudioParameterFloat("tremoloRate", 
                                             "Rate",
                                             juce::NormalisableRange<float>(0.1, 15.f, 0.01), 10.f);
        vector.emplace_back(rate);

        isAutoPan = new juce::AudioParameterBool("tremoloAutoPan", 
                                                 "Auto-Pan",
                                                 false);
        vector.emplace_back(isAutoPan);

        layout.add(vector.begin(), vector.end());
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};