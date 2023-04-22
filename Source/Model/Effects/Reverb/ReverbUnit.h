/*
==============================================================================

    Reverb.h
    Created: 14 Mar 2023 6:11:00pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessorUnit.h"

namespace Effects::Reverb
{
    using Reverb = juce::dsp::Reverb;

    static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> reverbGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>(
                "reverbGroup", 
                "Reverb", 
                "|"));

        auto wetLevel = std::make_unique<juce::AudioParameterFloat>(
            "reverbWet", 
            "Wet%",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            20.f);
        reverbGroup.get()->addChild(std::move(wetLevel));

        auto dryLevel = std::make_unique<juce::AudioParameterFloat>(
            "reverbDry", 
            "Dry%",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            80.f);
        reverbGroup.get()->addChild(std::move(dryLevel));

        auto roomSize = std::make_unique<juce::AudioParameterFloat>(
            "reverbRoom", 
            "Room Size",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            50.f);
        reverbGroup.get()->addChild(std::move(roomSize));

        auto damping = std::make_unique<juce::AudioParameterFloat>(
            "reverbDamping", 
            "Damping",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            50.f);
        reverbGroup.get()->addChild(std::move(damping));

        auto width = std::make_unique<juce::AudioParameterFloat>(
            "reverbWidth", 
            "Width",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            50.f);
        reverbGroup.get()->addChild(std::move(width));

        return reverbGroup;
    }

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
        
        EffectEditorUnit* createEditorUnit() override;

    private:
        Reverb reverb;

        void updateReverbParameters();     

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbUnit)
    };
}