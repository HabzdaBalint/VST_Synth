/*
==============================================================================

    FXDelayParameters.h
    Created: 17 Mar 2023 7:07:17pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

constexpr float DELAY_MAXLENGTH = 1000.f;

struct FXDelayParameters : public juce::AudioProcessorValueTreeState::Listener
{
    FXDelayParameters(std::function<void()> update) : update(update) {}

    std::function<void()> update = nullptr;

    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> delayGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("delayGroup", "Delay", "|"));

        auto mix = std::make_unique<juce::AudioParameterFloat>("delayMix",
                                            "Wet%",
                                            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 35.f);                                
        delayGroup.get()->addChild(std::move(mix));

        auto feedback = std::make_unique<juce::AudioParameterFloat>("delayFeedback",
                                                 "Feedback",
                                                 juce::NormalisableRange<float>(0.f, 100.f, 1.f), 40.f);
        delayGroup.get()->addChild(std::move(feedback));

        auto time = std::make_unique<juce::AudioParameterFloat>("delayTime",
                                             "Time",
                                             juce::NormalisableRange<float>(1.f, DELAY_MAXLENGTH, 0.1), 250.f);
        delayGroup.get()->addChild(std::move(time));
        
        auto filterFrequency = std::make_unique<juce::AudioParameterFloat>("delayFilterFrequency",
                                                        "Center Frequency",
                                                        juce::NormalisableRange<float>(10.f, 22000.f, 0.1, 0.3), 500.f);
        delayGroup.get()->addChild(std::move(filterFrequency));
        
        auto filterQ = std::make_unique<juce::AudioParameterFloat>("delayFilterQ",
                                                "Q",
                                                juce::NormalisableRange<float>(0.05, 5.f, 0.001), 0.5);
        delayGroup.get()->addChild(std::move(filterQ));
        
        return delayGroup;
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};