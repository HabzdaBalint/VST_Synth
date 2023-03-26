/*
==============================================================================

    FXReverbParameters.h
    Created: 17 Mar 2023 10:46:38pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct FXReverbParameters : public juce::AudioProcessorValueTreeState::Listener
{
    FXReverbParameters(std::function<void()> update) : update(update) {}

    std::function<void()> update = nullptr;

    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> reverbGroup (
                    std::make_unique<juce::AudioProcessorParameterGroup>("reverbGroup", "Reverb", "|"));
        auto wetLevel = std::make_unique<juce::AudioParameterFloat>("reverbWet", 
                                                 "Wet%",
                                                 juce::NormalisableRange<float>(0.f, 100.f, 0.1), 35.f);
        reverbGroup.get()->addChild(std::move(wetLevel));

        auto dryLevel = std::make_unique<juce::AudioParameterFloat>("reverbDry", 
                                                 "Dry%",
                                                 juce::NormalisableRange<float>(0.f, 100.f, 0.1), 100.f);
        reverbGroup.get()->addChild(std::move(dryLevel));

        auto roomSize = std::make_unique<juce::AudioParameterFloat>("reverbRoom", 
                                                 "Room Size",
                                                 juce::NormalisableRange<float>(0.f, 100.f, 0.1), 50.f);
        reverbGroup.get()->addChild(std::move(roomSize));

        auto damping = std::make_unique<juce::AudioParameterFloat>("reverbDamping", 
                                                "Damping",
                                                juce::NormalisableRange<float>(0.f, 100.f, 0.1), 50.f);
        reverbGroup.get()->addChild(std::move(damping));

        auto width = std::make_unique<juce::AudioParameterFloat>("reverbWidth", 
                                              "Width",
                                              juce::NormalisableRange<float>(0.f, 100.f, 0.1), 50.f);
        reverbGroup.get()->addChild(std::move(width));

        return reverbGroup;
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};