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

    std::function<void()> update = nullptr;

    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> phaserGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("phaserGroup", "Phaser", "|"));

        auto mix = std::make_unique<juce::AudioParameterFloat>("phaserMix", 
                                            "Wet%",
                                            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 35.f);
        phaserGroup.get()->addChild(std::move(mix));

        auto rate = std::make_unique<juce::AudioParameterFloat>("phaserRate", 
                                             "Rate",
                                             juce::NormalisableRange<float>(0.1, 25.f, 0.01), 10.f);
        phaserGroup.get()->addChild(std::move(rate));

        auto depth = std::make_unique<juce::AudioParameterFloat>("phaserDepth", 
                                              "Depth",
                                              juce::NormalisableRange<float>(0.f, 100.f, 0.1), 30.f);
        phaserGroup.get()->addChild(std::move(depth));

        auto frequency = std::make_unique<juce::AudioParameterFloat>("phaserFrequency", 
                                                  "Frequency",
                                                  juce::NormalisableRange<float>(10.f, 22000.f, 0.1, 0.3), 500.f);
        phaserGroup.get()->addChild(std::move(frequency));

        auto feedback = std::make_unique<juce::AudioParameterFloat>("phaserFeedback", 
                                                 "Feedback",
                                                 juce::NormalisableRange<float>(0.f, 100.f, 0.1), 40.f);
        phaserGroup.get()->addChild(std::move(feedback));

        return phaserGroup;
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};