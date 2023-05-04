/*
==============================================================================

    Tremolo.h
    Created: 19 Mar 2023 7:18:47pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessor.h"

namespace Processor::Effects::Tremolo
{
    static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> tremoloGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>(
                "tremoloGroup", 
                "Tremolo", 
                "|"));

        auto depth = std::make_unique<juce::AudioParameterFloat>(
            "tremoloDepth", 
            "Depth",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            50.f);
        tremoloGroup.get()->addChild(std::move(depth));

        auto rate = std::make_unique<juce::AudioParameterFloat>(
            "tremoloRate", 
            "Rate",
            juce::NormalisableRange<float>(0.1, 15.f, 0.01, 0.35), 
            2.f);
        tremoloGroup.get()->addChild(std::move(rate));

        auto isAutoPan = std::make_unique<juce::AudioParameterBool>(
            "tremoloAutoPan", 
            "Auto-Pan",
            false);
        tremoloGroup.get()->addChild(std::move(isAutoPan));

        return tremoloGroup;
    }
    
    class TremoloProcessor : public EffectProcessor
    {
    public:
        TremoloProcessor(juce::AudioProcessorValueTreeState& apvts);
        ~TremoloProcessor() override;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
        void releaseResources() override;
        
        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const;
        void removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const;
        
        Editor::Effects::EffectEditor* createEditorUnit() override;

        const juce::String getName() const override
        {
            return "Tremolo";
        }

    private:
        juce::AudioProcessorValueTreeState& apvts;

        float depth = 0;
        float rate = 0;
        bool isAutoPan = false;

        float currentAngle = 0;
        float angleDelta = 0;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void updateTremoloParameters();

        void updateAngles();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TremoloProcessor)
    };
}

