/*
  ==============================================================================

    FXProcessorChainParameters.h
    Created: 13 Mar 2023 11:51:05pm
    Author:  Habama10

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

constexpr int FX_MAX_SLOTS = 5;

struct FXProcessorChainParameters : juce::AudioProcessorValueTreeState::Listener
{
    FXProcessorChainParameters(std::function<void()> update) : update(update) {}

    juce::StringArray choices = {"Empty", "EQ", "Fliter", "Compressor", "Delay", "Reverb", "Chorus", "Phaser"};

    juce::AudioParameterBool *bypassProcessor[FX_MAX_SLOTS] = {};
    juce::AudioParameterChoice *choiceProcessor[FX_MAX_SLOTS] = {};

    std::function<void()> update = nullptr;

    void createParameterLayout(juce::AudioProcessorValueTreeState::ParameterLayout &layout)
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> vector;

        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            //Bypasses for each loaded Effect
            bypassProcessor[i] = new juce::AudioParameterBool(getFXBypassParameterName(i), "Bypass " + juce::String(i) , false);
            vector.emplace_back(bypassProcessor[i]);
            //Bypasses for each loaded Effect
            choiceProcessor[i] = new juce::AudioParameterChoice(getFXChoiceParameterName(i), "FX Slot " + juce::String(i), choices, 0);
            layout.add(vector.begin(), vector.end());
        }

        layout.add(vector.begin(), vector.end());
    }

    /// @brief Used for making the parameter ids of the the FX slots' bypass parameters consistent
    /// @param index The index of the effect
    /// @return A consistent parameter id
    juce::String getFXBypassParameterName(size_t index)
    {
        return "bypass" + juce::String(index);
    }

    /// @brief Used for making the parameter ids of the the FX slots' choice parameters consistent
    /// @param index The index of the effect
    /// @return A consistent parameter id
    juce::String getFXChoiceParameterName(size_t index)
    {
        return "fxSlot" + juce::String(index);
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};