/*
==============================================================================

    FXProcessorChainParameters.h
    Created: 13 Mar 2023 11:51:05pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

constexpr int FX_MAX_SLOTS = 8;

struct FXProcessorChainParameters : juce::AudioProcessorValueTreeState::Listener
{
    FXProcessorChainParameters(std::function<void()> update) : update(update) {}

    juce::StringArray choices = {"Empty", "EQ", "Fliter", "Compressor", "Delay", "Reverb", "Chorus", "Phaser", "Tremolo"};

    std::function<void()> update = nullptr;

    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> fxGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("fxGroup", "Effects", "|"));

        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            //Bypasses for each loaded Effect
            auto bypassProcessor = std::make_unique<juce::AudioParameterBool>(getFXBypassParameterName(i), "Bypass " + juce::String(i+1) , false);
            fxGroup.get()->addChild(std::move(bypassProcessor));
            
            //Choices for each loaded Effect
            auto choiceProcessor = std::make_unique<juce::AudioParameterChoice>(getFXChoiceParameterName(i), "FX Slot " + juce::String(i+1), choices, 0);
            fxGroup.get()->addChild(std::move(choiceProcessor));
        }

        return fxGroup;
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