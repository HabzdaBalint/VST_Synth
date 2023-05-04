/*
==============================================================================

    EffectProcessorChain.h
    Created: 12 Mar 2023 07:32:57pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Equalizer/EqualizerProcessor.h"
#include "Filter/FilterProcessor.h"
#include "Compressor/CompressorProcessor.h"
#include "Delay/DelayProcessor.h"
#include "Reverb/ReverbProcessor.h"
#include "Chorus/ChorusProcessor.h"
#include "Phaser/PhaserProcessor.h"
#include "Tremolo/TremoloProcessor.h"

namespace Processor::Effects::EffectsChain
{
    static const juce::StringArray chainChoices = { "Empty", "EQ", "Fliter", "Compressor", "Delay", "Reverb", "Chorus", "Phaser", "Tremolo" };
    enum EffectChoices { Empty = 0, EQ = 1, Filter = 2, Compressor = 3, Delay = 4, Reverb = 5, Chorus = 6, Phaser = 7, Tremolo = 8 };

    const int FX_MAX_SLOTS = chainChoices.size() - 1;

    struct EffectSlot
    {
        EffectSlot(bool bypass, std::shared_ptr<EffectProcessor> newProcessor) :
            bypass(bypass),
            processor(newProcessor)
        {}

        EffectSlot()
        {
            processor.store(std::make_shared<EffectProcessor>());
        }

        std::atomic<bool> bypass {false};
        std::atomic<std::shared_ptr<EffectProcessor>> processor;
    };

    /// @brief Used for making the parameter ids of the the FX slots' bypass parameters consistent
    /// @param index The index of the effect
    /// @return A parameter id
    static const juce::String getFXBypassParameterID(const int index)
    {
        jassert(juce::isPositiveAndBelow(index, FX_MAX_SLOTS));
        return "fxBypass" + juce::String(index);
    }

    static const int getFXIndexFromBypassParameterID(const juce::String paramID)
    {
        int index = std::stoi(paramID.trimCharactersAtStart("fxBypass").toStdString());
        jassert(juce::isPositiveAndBelow(index, FX_MAX_SLOTS));
        return index;
    }

    /// @brief Used for making the parameter ids of the the FX slots' choice parameters consistent
    /// @param index The index of the effect
    /// @return A parameter id
    static const juce::String getFXChoiceParameterID(const int index)
    {
        jassert(juce::isPositiveAndBelow(index, FX_MAX_SLOTS));
        return "fxChoice" + juce::String(index);
    }

    static const int getFXIndexFromChoiceParameterID(const juce::String paramID)
    {
        int index = std::stoi(paramID.trimCharactersAtStart("fxChoice").toStdString());
        jassert(juce::isPositiveAndBelow(index, FX_MAX_SLOTS));
        return index;
    }

    static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> fxChainGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("fxChainGroup", "Effect Chain", "|"));

        juce::AudioParameterChoiceAttributes attr;

        for(int i = 0; i < FX_MAX_SLOTS; i++)
        {
            //Bypasses for each loaded Effect
            auto processorBypass = std::make_unique<juce::AudioParameterBool>(
                getFXBypassParameterID(i),
                "Bypass " + juce::String(i+1),
                false);
            fxChainGroup.get()->addChild(std::move(processorBypass));
            
            //Choices for each loaded Effect
            auto processorChoice = std::make_unique<juce::AudioParameterChoice>(
                getFXChoiceParameterID(i),
                "FX Slot " + juce::String(i+1),
                chainChoices,
                0,
                attr.withAutomatable(false).withMeta(true));
            fxChainGroup.get()->addChild(std::move(processorChoice));
        }

        return fxChainGroup;
    }

    class EffectProcessorChain : public juce::AudioProcessor,
                                 public juce::AudioProcessorValueTreeState::Listener
    {
    public:
        EffectProcessorChain(juce::AudioProcessorValueTreeState&);

        ~EffectProcessorChain();

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

        void registerListener(juce::AudioProcessorValueTreeState::Listener*) const;
        void removeListener(juce::AudioProcessorValueTreeState::Listener*) const;
        
        const juce::Array<Editor::Effects::EffectEditor*> getLoadedEffectEditors() const;

        bool isProcessorInChain(const EffectProcessor& processor) const;

    private:
        juce::AudioProcessorValueTreeState& apvts;
        std::unordered_map<juce::String, std::atomic<float>> paramMap;

        juce::OwnedArray<EffectSlot> chain;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectProcessorChain)
    };
}