/*
==============================================================================

    ChorusProcessor.h
    Created: 14 Mar 2023 6:11:06pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessor.h"

namespace Effects::Chorus
{
    using Chorus = juce::dsp::Chorus<float>;

    static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> chorusGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>(
                "chorusGroup", 
                "Chorus", 
                "|"));

        auto mix = std::make_unique<juce::AudioParameterFloat>(
            "chorusMix", 
            "Wet%",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            35.f);
        chorusGroup.get()->addChild(std::move(mix));

        auto rate = std::make_unique<juce::AudioParameterFloat>(
            "chorusRate", 
            "Rate",
            juce::NormalisableRange<float>(0.1, 20.f, 0.01, 0.35), 
            3.f);
        chorusGroup.get()->addChild(std::move(rate));

        auto delay = std::make_unique<juce::AudioParameterFloat>(
            "chorusDelay", 
            "Delay",
            juce::NormalisableRange<float>(1.f, 50.f, 0.1), 
            20.f);
        chorusGroup.get()->addChild(std::move(delay));

        auto depth = std::make_unique<juce::AudioParameterFloat>(
            "chorusDepth", 
            "Depth",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            20.f);
        chorusGroup.get()->addChild(std::move(depth));

        auto feedback = std::make_unique<juce::AudioParameterFloat>(
            "chorusFeedback", 
            "Feedback",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            40.f);
        chorusGroup.get()->addChild(std::move(feedback));

        return chorusGroup;
    }

    class ChorusProcessor : public EffectProcessor
    {
    public:
        ChorusProcessor(juce::AudioProcessorValueTreeState& apvts);
        ~ChorusProcessor() override;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
        void releaseResources() override;

        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const;
        void removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const;
        
        EffectEditor* createEditorUnit() override;

    private:
        Chorus chorus;
        
        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void updateChorusParameters();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChorusProcessor)
    };
}