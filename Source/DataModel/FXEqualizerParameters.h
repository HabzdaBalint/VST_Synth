/*
==============================================================================

    FXEqualizerParameters.h
    Created: 15 Mar 2023 8:55:21pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct FXEqualizerParameters : juce::AudioProcessorValueTreeState::Listener
{
    FXEqualizerParameters(std::function<void()> update) : update(update) {}

    std::function<void()> update = nullptr;

    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> eqGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("eqGroup", "Equalizer", "|"));

        for (size_t i = 0; i < 10; i++)
        {
            auto bandGain = std::make_unique<juce::AudioParameterFloat>(getBandGainParameterName(i),
                                                            getBandFrequencyLabel(i),
                                                            juce::NormalisableRange<float>(-12.f, 12.f, 0.1), 0.f);
            eqGroup.get()->addChild(std::move(bandGain));
        }
        
        return eqGroup;
    }

    
    /// @brief Used for making the parameter ids of the the bands' gain parameters consistent
    /// @param index The index of the band
    /// @return A consistent parameter id
    juce::String getBandGainParameterName(size_t index)
    {
        return "band" + juce::String(index) + "gain";
    }

    /// @brief Used for getting usable frequency numbers from a bands' index
    /// @param index The index of the band
    /// @return A string containing the frequency the band is responsible for
    juce::String getBandFrequencyLabel(size_t index)
    {
        float frequency = 31.25 * pow(2, index);
        juce::String suffix;
        if(frequency >= 1000)
        {
            suffix = "kHz";
            frequency /= 1000;
        }
        else
        {
            suffix = "Hz";
        }
        juce::String label(frequency, 0, false);
        return label + suffix;
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};