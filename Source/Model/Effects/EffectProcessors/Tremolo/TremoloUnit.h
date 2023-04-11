/*
==============================================================================

    Tremolo.h
    Created: 19 Mar 2023 7:18:47pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessorUnit.h"

namespace Effects::EffectProcessors::Tremolo
{
    using DryWetMixer = juce::dsp::DryWetMixer<float>;

    class TremoloUnit : public EffectProcessorUnit
    {
    public:
        TremoloUnit(juce::AudioProcessorValueTreeState& apvts);
        ~TremoloUnit() override;

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

        float depth = 0;
        float rate = 0;
        bool isAutoPan = false;

        float currentAngle = 0;
        float angleDelta = 0;

        void updateTremoloParameters();

        void updateAngles();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TremoloUnit)
    };
}

