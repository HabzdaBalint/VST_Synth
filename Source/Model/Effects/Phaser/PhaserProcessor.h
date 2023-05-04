/*
==============================================================================

    PhaserProcessor.h
    Created: 14 Mar 2023 6:11:14pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessor.h"

namespace Processor::Effects::Phaser
{
    using Phaser = juce::dsp::Phaser<float>;

    static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> phaserGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>(
                "phaserGroup", 
                "Phaser", 
                "|"));

        auto mix = std::make_unique<juce::AudioParameterFloat>(
            "phaserMix", 
            "Wet%",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1),
            35.f);
        phaserGroup.get()->addChild(std::move(mix));

        auto rate = std::make_unique<juce::AudioParameterFloat>(
            "phaserRate", 
            "Rate",
            juce::NormalisableRange<float>(0.1, 20.f, 0.01, 0.35),
            2.f);
        phaserGroup.get()->addChild(std::move(rate));

        auto depth = std::make_unique<juce::AudioParameterFloat>(
            "phaserDepth", 
            "Depth",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            30.f);
        phaserGroup.get()->addChild(std::move(depth));

        auto frequency = std::make_unique<juce::AudioParameterFloat>(
            "phaserFrequency", 
            "Frequency",
            juce::NormalisableRange<float>(10.f, 22000.f, 0.1, 0.25),
            1000.f);
        phaserGroup.get()->addChild(std::move(frequency));

        auto feedback = std::make_unique<juce::AudioParameterFloat>(
            "phaserFeedback", 
            "Feedback",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1),
            40.f);
        phaserGroup.get()->addChild(std::move(feedback));

        return phaserGroup;
    }

    class PhaserProcessor : public EffectProcessor
    {
    public:
        PhaserProcessor(juce::AudioProcessorValueTreeState& apvts);
        ~PhaserProcessor() override;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
        void releaseResources() override;

        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const;
        void removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const;
        
        Editor::Effects::EffectEditor* createEditorUnit() override;

        const juce::String getName() const override
        {
            return "Phaser";
        }

    private:
        juce::AudioProcessorValueTreeState& apvts;
        
        Phaser phaser;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void updatePhaserParameters();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaserProcessor)
    };
}