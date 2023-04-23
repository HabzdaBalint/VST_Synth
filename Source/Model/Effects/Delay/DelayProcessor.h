/*
==============================================================================

    DelayProcessor.h
    Created: 14 Mar 2023 6:12:18pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessor.h"

namespace Effects::Delay
{
    using Delay = juce::dsp::DelayLine<float>;
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    using DryWetMixer = juce::dsp::DryWetMixer<float>;

    constexpr float MAX_LENGTH_MS = 1000.f;

    static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> delayGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>(
                "delayGroup", 
                "Delay", 
                "|"));

        auto mix = std::make_unique<juce::AudioParameterFloat>(
            "delayMix",
            "Wet%",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            35.f);                                
        delayGroup.get()->addChild(std::move(mix));

        auto feedback = std::make_unique<juce::AudioParameterFloat>(
            "delayFeedback",
            "Feedback",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            40.f);
        delayGroup.get()->addChild(std::move(feedback));

        auto time = std::make_unique<juce::AudioParameterFloat>(
            "delayTime",
            "Time",
            juce::NormalisableRange<float>(1.f, MAX_LENGTH_MS, 0.1, 0.5),
            250.f);
        delayGroup.get()->addChild(std::move(time));
        
        auto filterFrequency = std::make_unique<juce::AudioParameterFloat>(
            "delayFilterFrequency",
            "Center Frequency",
            juce::NormalisableRange<float>(10.f, 22000.f, 0.1, 0.25),
            500.f);
        delayGroup.get()->addChild(std::move(filterFrequency));
        
        auto filterQ = std::make_unique<juce::AudioParameterFloat>(
            "delayFilterQ",
            "Q",
            juce::NormalisableRange<float>(0.05, 5.f, 0.001, 0.4),
            0.5);
        delayGroup.get()->addChild(std::move(filterQ));
        
        return delayGroup;
    }

    class DelayProcessor : public EffectProcessor
    {
    public:
        DelayProcessor(juce::AudioProcessorValueTreeState& apvts);
        ~DelayProcessor();

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
        void releaseResources() override;

        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const;
        void removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const;

        EffectEditor* createEditorUnit() override;

    private:
        DryWetMixer dryWetMixer;
        Delay delay;
        juce::OwnedArray<Filter> filters;

        float feedback = 0;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void updateDelayParameters();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayProcessor)
    };
}