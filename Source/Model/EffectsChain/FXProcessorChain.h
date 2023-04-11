/*
==============================================================================

    FXProcessorChainParameters.h
    Created: 12 Mar 2023 07:32:57pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "../EffectProcessors/EqualizerUnit.h"
#include "../EffectProcessors/FilterUnit.h"
#include "../EffectProcessors/CompressorUnit.h"
#include "../EffectProcessors/DelayUnit.h"
#include "../EffectProcessors/ReverbUnit.h"
#include "../EffectProcessors/ChorusUnit.h"
#include "../EffectProcessors/PhaserUnit.h"
#include "../EffectProcessors/TremoloUnit.h"

namespace EffectsChain
{
    static const juce::StringArray choices = { "Empty", "EQ", "Fliter", "Compressor", "Delay", "Reverb", "Chorus", "Phaser", "Tremolo" };
    const int FX_MAX_SLOTS = choices.size() - 1;

    struct FXProcessor
    {
        FXProcessor(bool bypass, std::shared_ptr<juce::AudioProcessor> processor) :
            bypass(bypass),
            processor(processor)
        {}

        FXProcessor() {}

        std::atomic<bool> bypass{false};
        std::atomic<std::shared_ptr<juce::AudioProcessor>> processor{nullptr};
    };

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
        bool hasEditor() const override { return true; }

        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const juce::String getProgramName(int) override { return {}; }
        void changeProgramName(int, const juce::String&) override {}

        void getStateInformation(juce::MemoryBlock&) override {}
        void setStateInformation(const void*, int) override {}

        double getTailLengthSeconds() const override { return 0; }

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;
        void processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &midiMessages) override;

        void registerListener(juce::AudioProcessorValueTreeState::Listener*);
        void removeListener(juce::AudioProcessorValueTreeState::Listener*);

        static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout();

        /// @brief Used for making the parameter ids of the the FX slots' bypass parameters consistent
        /// @param index The index of the effect
        /// @return A consistent parameter id
        static const juce::String getFXBypassParameterName(size_t index);

        /// @brief Used for making the parameter ids of the the FX slots' choice parameters consistent
        /// @param index The index of the effect
        /// @return A consistent parameter id
        static const juce::String getFXChoiceParameterName(size_t index);

    private:
        juce::AudioProcessorValueTreeState& apvts;
               
        juce::OwnedArray<FXProcessor> chain;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXProcessorChain)
    };
} // namespace FXChain