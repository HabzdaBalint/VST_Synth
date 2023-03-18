/*
==============================================================================

    FXPhaserParameters.h
    Created: 18 Mar 2023 3:11:00pm
    Author:  Habama10
    
==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct FXPhaserParameters : public juce::AudioProcessorValueTreeState::Listener
{
    FXPhaserParameters(std::function<void()> update) : update(update) {}

    juce::AudioParameterFloat* mix = nullptr;
    juce::AudioParameterFloat* rate = nullptr;
    juce::AudioParameterFloat* depth = nullptr;
    juce::AudioParameterFloat* frequency = nullptr;
    juce::AudioParameterFloat* feedback  = nullptr; 

    std::function<void()> update = nullptr;

    void createParameterLayout(juce::AudioProcessorValueTreeState::ParameterLayout &layout)
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> vector;

        mix = new juce::AudioParameterFloat("phaserMix", 
                                            "Wet%",
                                            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 35.f);
        vector.emplace_back(mix);

        rate = new juce::AudioParameterFloat("phaserRate", 
                                             "Rate",
                                             juce::NormalisableRange<float>(0.1, 30.f, 0.01), 10.f);
        vector.emplace_back(rate);

        depth = new juce::AudioParameterFloat("phaserDepth", 
                                              "Depth",
                                              juce::NormalisableRange<float>(0.f, 100.f, 0.1), 30.f);
        vector.emplace_back(depth);

        frequency = new juce::AudioParameterFloat("phaserFrequency", 
                                                  "Frequency",
                                                  juce::NormalisableRange<float>(10.f, 22000.f, 0.1, 0.3), 500.f);
        vector.emplace_back(frequency);

        feedback = new juce::AudioParameterFloat("phaserFeedback", 
                                                 "Feedback",
                                                 juce::NormalisableRange<float>(-100.f, 100.f, 0.1), 40.f);
        vector.emplace_back(feedback);

        layout.add(vector.begin(), vector.end());
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};