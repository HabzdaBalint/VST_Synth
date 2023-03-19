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

    juce::AudioParameterFloat* mix;
    juce::AudioParameterFloat* feedback;
    juce::AudioParameterFloat* time;
    juce::AudioParameterFloat* filterFrequency;
    juce::AudioParameterFloat* filterQ;

    std::function<void()> update = nullptr;

    void createParameterLayout(juce::AudioProcessorValueTreeState::ParameterLayout &layout)
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> vector;

        mix = new juce::AudioParameterFloat("delayMix",
                                            "Wet%",
                                            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 35.f);
        vector.emplace_back(mix);

        feedback = new juce::AudioParameterFloat("delayFeedback",
                                                 "Feedback",
                                                 juce::NormalisableRange<float>(0.f, 100.f, 1.f), 40.f);
        vector.emplace_back(feedback);

        time = new juce::AudioParameterFloat("delayTime",
                                             "Time",
                                             juce::NormalisableRange<float>(1.f, DELAY_MAXLENGTH, 0.1), 250.f);
        vector.emplace_back(time);

        filterFrequency = new juce::AudioParameterFloat("delayFilterFrequency",
                                                        "Center Frequency",
                                                        juce::NormalisableRange<float>(10.f, 22000.f, 0.1, 0.3), 500.f);
        vector.emplace_back(filterFrequency);

        filterQ = new juce::AudioParameterFloat("delayFilterQ",
                                                "Q",
                                                juce::NormalisableRange<float>(0.05, 5.f, 0.001), 0.5);
        vector.emplace_back(filterQ);

        layout.add(vector.begin(), vector.end());
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};