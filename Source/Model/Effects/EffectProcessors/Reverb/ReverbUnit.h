/*
==============================================================================

    Reverb.h
    Created: 14 Mar 2023 6:11:00pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessorUnit.h"

namespace Effects::EffectProcessors::Reverb
{
    using Reverb = juce::dsp::Reverb;

    class ReverbUnit : public EffectProcessorUnit
    {
    public:
        ReverbUnit(juce::AudioProcessorValueTreeState& apvts);
        ~ReverbUnit() override;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
        void releaseResources() override;

        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener);
        void removeListener(juce::AudioProcessorValueTreeState::Listener* listener);

        void parameterChanged(const juce::String &parameterID, float newValue) override;
        static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout();
        
        EffectEditorUnit* createEditorUnit() override;

    private:
        Reverb reverb;

        void updateReverbParameters();     

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbUnit)
    };
}