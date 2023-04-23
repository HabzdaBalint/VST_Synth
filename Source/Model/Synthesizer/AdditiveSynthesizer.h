/*
==============================================================================

    AdditiveSynthesizer.h
    Created: 5 Mar 2023 15:47:05am
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "AdditiveSynthParameters.h"
#include "AdditiveSound.h"
#include "AdditiveVoice.h"

namespace Synthesizer
{
    using Gain = juce::dsp::Gain<float>;

    class AdditiveSynthesizer : public juce::AudioProcessor
    {
    public:
        AdditiveSynthesizer(juce::AudioProcessorValueTreeState&);
        ~AdditiveSynthesizer() override;

        const juce::String getName() const override { return "Additive Synthesizer"; }
        bool acceptsMidi() const override { return true; }
        bool producesMidi() const override { return false; }

        juce::AudioProcessorEditor* createEditor() override { return nullptr; };
        bool hasEditor() const override { return false; }

        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const juce::String getProgramName(int) override { return {}; }
        void changeProgramName(int, const juce::String &) override {}

        void getStateInformation(juce::MemoryBlock& destData) override {}
        void setStateInformation(const void* data, int sizeInBytes) override {}
        double getTailLengthSeconds() const override { return synthParameters.amplitudeADSRRelease->load() / 1000; }

        void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
        void releaseResources() override {};
        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;

        const OscillatorParameters& getOscParameters() const
        {
            return oscParameters;
        }

        const AdditiveSynthParameters& getSynthParameters() const
        {
            return synthParameters;
        }

    private:
        AdditiveSynthParameters synthParameters;
        OscillatorParameters oscParameters;

        Gain synthGain;
        juce::Synthesiser synth;
        
        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthesizer)
    };
}