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

    juce::AudioParameterFloat* dryWetMix;
    juce::AudioParameterFloat* threshold;
    juce::AudioParameterFloat* ratio;
    juce::AudioParameterFloat* attack;
    juce::AudioParameterFloat* release;

    std::function<void()> update = nullptr;

    void createParameterLayout(juce::AudioProcessorValueTreeState::ParameterLayout &layout)
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> vector;

        dryWetMix = new juce::AudioParameterFloat("compressorMix",
                                                  "Wet%",
                                                  juce::NormalisableRange<float>(0.f, 100.f, 0.1), 100.f);
        vector.emplace_back(dryWetMix);

        threshold = new juce::AudioParameterFloat("compressorThreshold",
                                                  "Threshold",
                                                  juce::NormalisableRange<float>(-60.f, 0.f, 0.01), -18.f);
        vector.emplace_back(threshold);

        ratio = new juce::AudioParameterFloat("compressorRatio",
                                              "Ratio",
                                              juce::NormalisableRange<float>(1.f, 100.f, 0.01), 4.f);
        vector.emplace_back(ratio);

        attack = new juce::AudioParameterFloat("compressorAttack",
                                               "Attack",
                                               juce::NormalisableRange<float>(0.01, 250.f, 0.01), 0.5);
        vector.emplace_back(attack);

        release = new juce::AudioParameterFloat("compressorRelease",
                                                "Release",
                                                juce::NormalisableRange<float>(10.f, 1000.f, 0.1), 250.f);
        vector.emplace_back(release);

        layout.add(vector.begin(), vector.end());
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};