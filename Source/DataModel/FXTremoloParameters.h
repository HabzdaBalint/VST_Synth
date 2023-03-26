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

    std::function<void()> update = nullptr;

    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> tremoloGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("tremoloGroup", "Tremolo", "|"));

        auto mix = std::make_unique<juce::AudioParameterFloat>("tremoloMix", 
                                            "Wet%",
                                            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 100.f);
        tremoloGroup.get()->addChild(std::move(mix));

        auto depth = std::make_unique<juce::AudioParameterFloat>("tremoloDepth", 
                                              "Depth",
                                              juce::NormalisableRange<float>(0.f, 100.f, 0.1), 50.f);
        tremoloGroup.get()->addChild(std::move(depth));

        auto rate = std::make_unique<juce::AudioParameterFloat>("tremoloRate", 
                                             "Rate",
                                             juce::NormalisableRange<float>(0.1, 15.f, 0.01), 10.f);
        tremoloGroup.get()->addChild(std::move(rate));

        auto isAutoPan = std::make_unique<juce::AudioParameterBool>("tremoloAutoPan", 
                                                 "Auto-Pan",
                                                 false);
        tremoloGroup.get()->addChild(std::move(isAutoPan));

        return tremoloGroup;
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};