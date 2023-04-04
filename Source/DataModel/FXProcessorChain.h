/*
==============================================================================

    FXProcessorChainParameters.h
    Created: 12 Mar 2023 07:32:57pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FXEqualizer.h"
#include "FXFilter.h"
#include "FXCompressor.h"
#include "FXDelay.h"
#include "FXReverb.h"
#include "FXChorus.h"
#include "FXPhaser.h"
#include "FXTremolo.h"

namespace FXChain
{
    static const juce::StringArray choices = { "Empty", "EQ", "Fliter", "Compressor", "Delay", "Reverb", "Chorus", "Phaser", "Tremolo"};
    const int FX_MAX_SLOTS = choices.size() - 1;

    class FXProcessorChain : public juce::AudioProcessor,
                             public juce::AudioProcessorValueTreeState::Listener
    {
    public:
        FXProcessorChain(juce::AudioProcessorValueTreeState&);

        ~FXProcessorChain();

        const juce::String getName() const override { return "Effect Chain"; }
        bool acceptsMidi() const override { return true; }
        bool producesMidi() const override { return true; }

        juce::AudioProcessorEditor* createEditor() { return nullptr; }
        bool hasEditor() const override { return true; }

        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const juce::String getProgramName(int) override { return {}; }
        void changeProgramName(int, const juce::String&) override {}

        void getStateInformation(juce::MemoryBlock&) override {}
        void setStateInformation(const void*, int) override {}

        double getTailLengthSeconds() const override;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;
        void processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &midiMessages) override;

        void registerListeners();

        static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout();

        /// @brief Used for making the parameter ids of the the FX slots' bypass parameters consistent
        /// @param index The index of the effect
        /// @return A consistent parameter id
        static const juce::String getFXBypassParameterName(size_t index)
        {
            return "bypass" + juce::String(index);
        }

        /// @brief Used for making the parameter ids of the the FX slots' choice parameters consistent
        /// @param index The index of the effect
        /// @return A consistent parameter id
        static const juce::String getFXChoiceParameterName(size_t index)
        {
            return "fxSlot" + juce::String(index);
        }

    private:
        juce::AudioProcessorValueTreeState& apvts;
        
        std::vector<std::unique_ptr<juce::AudioProcessor>> processors;
        std::vector<bool> bypasses;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXProcessorChain)
    };
} // namespace FXChain