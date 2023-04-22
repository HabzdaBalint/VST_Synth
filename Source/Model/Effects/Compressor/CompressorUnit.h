/*
==============================================================================

    CompressorUnit.h
    Created: 14 Mar 2023 6:12:03pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessorUnit.h"

namespace Effects::Compressor
{
    using Compressor = juce::dsp::Compressor<float>;
    using DryWetMixer = juce::dsp::DryWetMixer<float>;

    static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> compressorGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>(
                "compressorGroup", 
                "Compressor", 
                "|"));

        auto dryWetMix = std::make_unique<juce::AudioParameterFloat>(
            "compressorMix",
            "Wet%",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            100.f);
        compressorGroup.get()->addChild(std::move(dryWetMix));

        auto threshold = std::make_unique<juce::AudioParameterFloat>(
            "compressorThreshold",
            "Threshold",
            juce::NormalisableRange<float>(-60.f, 0.f, 0.01), 
            -18.f);
        compressorGroup.get()->addChild(std::move(threshold));

        juce::AudioParameterFloatAttributes attr;
        auto ratio = std::make_unique<juce::AudioParameterFloat>(
            "compressorRatio",
            "Ratio",
            juce::NormalisableRange<float>(1.f, 100.f, 0.01, 0.3), 
            4.f,
            attr.withStringFromValueFunction([](float value, int maximumStringLength)
                {
                    juce::String string = juce::String(value) + ":1";
                    return string;
                }));
        compressorGroup.get()->addChild(std::move(ratio));

        auto attack = std::make_unique<juce::AudioParameterFloat>(
            "compressorAttack",
            "Attack",
            juce::NormalisableRange<float>(0.01, 50.f, 0.01, 0.5), 
            0.5);
        compressorGroup.get()->addChild(std::move(attack));

        auto release = std::make_unique<juce::AudioParameterFloat>(
            "compressorRelease",
            "Release",
            juce::NormalisableRange<float>(1.f, 1000.f, 0.1, 0.5), 
            100.f);
        compressorGroup.get()->addChild(std::move(release));

        return compressorGroup;
    }

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
        
        EffectEditorUnit* createEditorUnit() override;

    private:
        DryWetMixer dryWetMixer;
        Compressor compressor;

        void updateCompressorParameters();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorUnit)
    };
}