/*
==============================================================================

    CompressorUnit.h
    Created: 14 Mar 2023 6:12:03pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessorUnit.h"

namespace Effects::EffectProcessors::Compressor
{
    using Compressor = juce::dsp::Compressor<float>;
    using DryWetMixer = juce::dsp::DryWetMixer<float>;

    class CompressorUnit : public EffectProcessorUnit
    {
    public:
        CompressorUnit(juce::AudioProcessorValueTreeState& apvts);
        ~CompressorUnit() override ;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
        void releaseResources() override;

        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener);
        void removeListener(juce::AudioProcessorValueTreeState::Listener* listener);

        void parameterChanged(const juce::String &parameterID, float newValue) override;
        static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout();
        
        EffectEditorUnit* createEditorUnit() override;

    private:
        DryWetMixer dryWetMixer;
        Compressor compressor;

        void updateCompressorParameters();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorUnit)
    };
}