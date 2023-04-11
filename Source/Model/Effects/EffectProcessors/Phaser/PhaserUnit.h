/*
==============================================================================

    PhaserUnit.h
    Created: 14 Mar 2023 6:11:14pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessorUnit.h"

namespace Effects::EffectProcessors::Phaser
{
    using Phaser = juce::dsp::Phaser<float>;

    class PhaserUnit : public EffectProcessorUnit
    {
    public:
        PhaserUnit(juce::AudioProcessorValueTreeState& apvts);
        ~PhaserUnit() override;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
        void releaseResources() override;

        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener);
        void removeListener(juce::AudioProcessorValueTreeState::Listener* listener);

        void parameterChanged(const juce::String &parameterID, float newValue) override;
        static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout();
        
        EffectEditorUnit* createEditorUnit() override;

    private:
        Phaser phaser;

        void updatePhaserParameters();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaserUnit)
    };
}