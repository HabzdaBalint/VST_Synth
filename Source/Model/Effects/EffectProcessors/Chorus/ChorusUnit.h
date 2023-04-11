/*
==============================================================================

    ChorusUnit.h
    Created: 14 Mar 2023 6:11:06pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessorUnit.h"

namespace Effects::EffectProcessors::Chorus
{
    using Chorus = juce::dsp::Chorus<float>;

    class ChorusUnit : public EffectProcessorUnit
    {
    public:
        ChorusUnit(juce::AudioProcessorValueTreeState& apvts);
        ~ChorusUnit() override;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
        void releaseResources() override;

        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener);
        void removeListener(juce::AudioProcessorValueTreeState::Listener* listener);

        void parameterChanged(const juce::String &parameterID, float newValue) override;
        
        static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout();
        
        EffectEditorUnit* createEditorUnit() override;

    private:
        Chorus chorus;
        
        void updateChorusParameters();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChorusUnit)
    };
}