/*
==============================================================================

    FXChorusParameters.h
    Created: 18 Mar 2023 3:11:19pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct FXChorusParameters : public juce::AudioProcessorValueTreeState::Listener
{
    FXChorusParameters(std::function<void()> update) : update(update) {}

    juce::AudioParameterFloat* mix = nullptr;
    juce::AudioParameterFloat* rate = nullptr;
    juce::AudioParameterFloat* delay = nullptr;
    juce::AudioParameterFloat* depth = nullptr;
    juce::AudioParameterFloat* feedback  = nullptr; 

    std::function<void()> update = nullptr;

    void createParameterLayout(juce::AudioProcessorValueTreeState::ParameterLayout &layout)
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> vector;

        mix = new juce::AudioParameterFloat("chorusMix", 
                                            "Wet%",
                                            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 35.f);
        vector.emplace_back(mix);

        rate = new juce::AudioParameterFloat("chorusRate", 
                                             "Rate",
                                             juce::NormalisableRange<float>(0.1, 25.f, 0.01), 10.f);
        vector.emplace_back(rate);

        delay = new juce::AudioParameterFloat("chorusDelay", 
                                              "Delay",
                                              juce::NormalisableRange<float>(1.f, 100.f, 0.1), 50.f);
        vector.emplace_back(delay);

        depth = new juce::AudioParameterFloat("chorusDepth", 
                                              "Depth",
                                              juce::NormalisableRange<float>(0.f, 100.f, 0.1), 30.f);
        vector.emplace_back(depth);

        feedback = new juce::AudioParameterFloat("chorusFeedback", 
                                                 "Feedback",
                                                 juce::NormalisableRange<float>(0.f, 100.f, 0.1), 20.f);
        vector.emplace_back(feedback);

        layout.add(vector.begin(), vector.end());
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};