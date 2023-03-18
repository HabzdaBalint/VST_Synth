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

    juce::AudioParameterFloat* wetLevel = nullptr;
    juce::AudioParameterFloat* dryLevel = nullptr;
    juce::AudioParameterFloat* roomSize = nullptr;
    juce::AudioParameterFloat* damping = nullptr;
    juce::AudioParameterFloat* width  = nullptr; 

    std::function<void()> update = nullptr;

    void createParameterLayout(juce::AudioProcessorValueTreeState::ParameterLayout &layout)
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> vector;

        wetLevel = new juce::AudioParameterFloat("reverbWet", 
                                                 "Wet%",
                                                 juce::NormalisableRange<float>(0.f, 100.f, 0.1), 35.f);
        vector.emplace_back(wetLevel);

        dryLevel = new juce::AudioParameterFloat("reverbDry", 
                                                 "Dry%",
                                                 juce::NormalisableRange<float>(0.f, 100.f, 0.1), 100.f);
        vector.emplace_back(dryLevel);

        roomSize = new juce::AudioParameterFloat("reverbRoom", 
                                                 "Room Size",
                                                 juce::NormalisableRange<float>(0.f, 100.f, 0.1), 50.f);
        vector.emplace_back(roomSize);

        damping = new juce::AudioParameterFloat("reverbDamping", 
                                                "Damping",
                                                juce::NormalisableRange<float>(0.f, 100.f, 0.1), 50.f);
        vector.emplace_back(damping);

        width = new juce::AudioParameterFloat("reverbWidth", 
                                              "Width",
                                              juce::NormalisableRange<float>(0.f, 100.f, 0.1), 50.f);
        vector.emplace_back(width);

        layout.add(vector.begin(), vector.end());
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};