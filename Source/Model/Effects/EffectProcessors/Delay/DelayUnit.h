/*
==============================================================================

    DelayUnit.h
    Created: 14 Mar 2023 6:12:18pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessorUnit.h"

namespace Effects::EffectProcessors::Delay
{
    using Delay = juce::dsp::DelayLine<float>;
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    using DryWetMixer = juce::dsp::DryWetMixer<float>;

    constexpr float MAX_LENGTH_MS = 1000.f;

    class DelayUnit : public EffectProcessorUnit
    {
    public:
        DelayUnit(juce::AudioProcessorValueTreeState& apvts);
        ~DelayUnit();

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
        Delay delay;
        juce::OwnedArray<Filter> filters;

        float feedback = 0;

        void updateDelayParameters();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayUnit)
    };
}