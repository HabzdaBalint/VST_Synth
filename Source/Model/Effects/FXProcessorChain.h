/*
==============================================================================

    FXProcessorChainParameters.h
    Created: 12 Mar 2023 07:32:57pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Equalizer/EqualizerUnit.h"
#include "Filter/FilterUnit.h"
#include "Compressor/CompressorUnit.h"
#include "Delay/DelayUnit.h"
#include "Reverb/ReverbUnit.h"
#include "Chorus/ChorusUnit.h"
#include "Phaser/PhaserUnit.h"
#include "Tremolo/TremoloUnit.h"

namespace Effects::EffectsChain
{
    static const juce::StringArray choices = { "Empty", "EQ", "Fliter", "Compressor", "Delay", "Reverb", "Chorus", "Phaser", "Tremolo" };
    const int FX_MAX_SLOTS = choices.size() - 1;

    struct FXProcessor
    {
        FXProcessor(bool bypass, std::shared_ptr<Effects::EffectProcessorUnit> processor) :
            bypass(bypass),
            processor(processor)
        {}

        FXProcessor() {}

        std::atomic<bool> bypass{false};
        std::atomic<std::shared_ptr<Effects::EffectProcessorUnit>> processor{nullptr};
    };

    /// @brief Used for making the parameter ids of the the FX slots' bypass parameters consistent
    /// @param index The index of the effect
    /// @return A parameter id
    static const juce::String getFXBypassParameterName(size_t index)
    {
        return "fxBypass" + juce::String(index);
    }

    /// @brief Used for making the parameter ids of the the FX slots' choice parameters consistent
    /// @param index The index of the effect
    /// @return A parameter id
    static const juce::String getFXChoiceParameterName(size_t index)
    {
        return "fxChoice" + juce::String(index);
    }
    
    static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> fxChainGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("fxChainGroup", "Effect Chain", "|"));

        juce::AudioParameterChoiceAttributes attr;

        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            //Bypasses for each loaded Effect
            auto processorBypass = std::make_unique<juce::AudioParameterBool>(
                getFXBypassParameterName(i),
                "Bypass " + juce::String(i+1),
                false);
            fxChainGroup.get()->addChild(std::move(processorBypass));
            
            //Choices for each loaded Effect
            auto processorChoice = std::make_unique<juce::AudioParameterChoice>(
                getFXChoiceParameterName(i),
                "FX Slot " + juce::String(i+1),
                choices,
                0,
                attr.withAutomatable(false).withMeta(true));
            fxChainGroup.get()->addChild(std::move(processorChoice));
        }

        return fxChainGroup;
    }

    class FXProcessorChain : public juce::AudioProcessor,
                             public juce::AudioProcessorValueTreeState::Listener
    {
    public:
        FXProcessorChain(juce::AudioProcessorValueTreeState&);

        ~FXProcessorChain();

        const juce::String getName() const override { return "Effect Chain"; }
        bool acceptsMidi() const override { return true; }
        bool producesMidi() const override { return true; }

        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }

        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const juce::String getProgramName(int) override { return {}; }
        void changeProgramName(int, const juce::String&) override {}

        void getStateInformation(juce::MemoryBlock&) override {}
        void setStateInformation(const void*, int) override {}

        double getTailLengthSeconds() const override { return 3; }

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;
        void processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &midiMessages) override;

        void registerListener(juce::AudioProcessorValueTreeState::Listener*);
        void removeListener(juce::AudioProcessorValueTreeState::Listener*);

        juce::Array<EffectEditorUnit*> getLoadedEffectEditors();

    private:
        juce::AudioProcessorValueTreeState& apvts;
               
        juce::OwnedArray<FXProcessor> chain;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXProcessorChain)
    };
}