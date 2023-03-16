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

    juce::AudioParameterFloat *bandGain[10] = {};

    std::function<void()> update = nullptr;

    void createParameterLayout(juce::AudioProcessorValueTreeState::ParameterLayout &layout)
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> vector;

        for (size_t i = 0; i < 10; i++)
        {
            bandGain[i] = new juce::AudioParameterFloat(getBandGainParameterName(i),
                                                        getBandFrequencyLabel(i),
                                                        juce::NormalisableRange<float>(-12.f, 12.f, 0.1), 0.f);
            vector.emplace_back(bandGain[i]);            
        }
        
        layout.add(vector.begin(), vector.end());
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