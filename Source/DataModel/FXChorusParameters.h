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

    std::function<void()> update = nullptr;

    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> chorusGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("chorusGroup", "Chorus", "|"));

        auto mix = std::make_unique<juce::AudioParameterFloat>("chorusMix", 
                                            "Wet%",
                                            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 35.f);
        chorusGroup.get()->addChild(std::move(mix));

        auto rate = std::make_unique<juce::AudioParameterFloat>("chorusRate", 
                                             "Rate",
                                             juce::NormalisableRange<float>(0.1, 25.f, 0.01), 10.f);
        chorusGroup.get()->addChild(std::move(rate));

        auto delay = std::make_unique<juce::AudioParameterFloat>("chorusDelay", 
                                              "Delay",
                                              juce::NormalisableRange<float>(1.f, 100.f, 0.1), 50.f);
        chorusGroup.get()->addChild(std::move(delay));

        auto depth = std::make_unique<juce::AudioParameterFloat>("chorusDepth", 
                                              "Depth",
                                              juce::NormalisableRange<float>(0.f, 100.f, 0.1), 30.f);
        chorusGroup.get()->addChild(std::move(depth));

        auto feedback = std::make_unique<juce::AudioParameterFloat>("chorusFeedback", 
                                                 "Feedback",
                                                 juce::NormalisableRange<float>(0.f, 100.f, 0.1), 20.f);
        chorusGroup.get()->addChild(std::move(feedback));

        return chorusGroup;
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};