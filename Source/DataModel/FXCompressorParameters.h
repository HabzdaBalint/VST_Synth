/*
    ==============================================================================

    FXCompressorParameters.h
    Created: 17 Mar 2023 5:04:42pm
    Author:  Habama10

    ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct FXCompressorParameters : juce::AudioProcessorValueTreeState::Listener
{
    FXCompressorParameters(std::function<void()> update) : update(update) {}

    std::function<void()> update = nullptr;

    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> compressorGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("compressorGroup", "Compressor", "|"));

        auto dryWetMix = std::make_unique<juce::AudioParameterFloat>("compressorMix",
                                                  "Wet%",
                                                  juce::NormalisableRange<float>(0.f, 100.f, 0.1), 100.f);
        compressorGroup.get()->addChild(std::move(dryWetMix));

        auto threshold = std::make_unique<juce::AudioParameterFloat>("compressorThreshold",
                                                  "Threshold",
                                                  juce::NormalisableRange<float>(-60.f, 0.f, 0.01), -18.f);
        compressorGroup.get()->addChild(std::move(threshold));

        auto ratio = std::make_unique<juce::AudioParameterFloat>("compressorRatio",
                                              "Ratio",
                                              juce::NormalisableRange<float>(1.f, 100.f, 0.01), 4.f);
        compressorGroup.get()->addChild(std::move(ratio));

        auto attack = std::make_unique<juce::AudioParameterFloat>("compressorAttack",
                                               "Attack",
                                               juce::NormalisableRange<float>(0.01, 250.f, 0.01), 0.5);
        compressorGroup.get()->addChild(std::move(attack));

        auto release = std::make_unique<juce::AudioParameterFloat>("compressorRelease",
                                                "Release",
                                                juce::NormalisableRange<float>(10.f, 1000.f, 0.1), 250.f);
        compressorGroup.get()->addChild(std::move(release));

        return compressorGroup;
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};