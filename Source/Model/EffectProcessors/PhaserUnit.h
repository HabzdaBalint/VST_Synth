/*
==============================================================================

    PhaserUnit.h
    Created: 14 Mar 2023 6:11:14pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorUnit.h"

namespace EffectProcessors::Phaser
{
    using Phaser = juce::dsp::Phaser<float>;

    class PhaserUnit : public FXProcessorUnit
    {
    public:
        PhaserUnit(juce::AudioProcessorValueTreeState& apvts) : FXProcessorUnit(apvts)
        {
            registerListener(this);
        }

        ~PhaserUnit() override
        {
            removeListener(this);
        }

        void prepareToPlay(double sampleRate, int samplesPerBlock) override
        {
            setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);

            juce::dsp::ProcessSpec processSpec;
            processSpec.maximumBlockSize = samplesPerBlock;
            processSpec.numChannels = getTotalNumOutputChannels();
            processSpec.sampleRate = sampleRate;
            phaser.prepare(processSpec);
            updatePhaserParameters();
        }

        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override
        {
            juce::dsp::AudioBlock<float> audioBlock(buffer);
            
            juce::dsp::ProcessContextReplacing<float> context(audioBlock);

            phaser.process(context);
        }

        void releaseResources() override
        {
            phaser.reset();
        }

        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener)
        {
            auto paramLayoutSchema = createParameterLayout();
            auto params = paramLayoutSchema->getParameters(false);

            for(auto param : params)
            {
                auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
                apvts.addParameterListener(id, listener);
            }
        }

        void removeListener(juce::AudioProcessorValueTreeState::Listener* listener)
        {
            auto paramLayoutSchema = createParameterLayout();
            auto params = paramLayoutSchema->getParameters(false);

            for(auto param : params)
            {
                auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
                apvts.removeParameterListener(id, listener);
            }
        }

        void updatePhaserParameters()
        {
            phaser.setMix(apvts.getRawParameterValue("phaserMix")->load()/100);
            phaser.setRate(apvts.getRawParameterValue("phaserRate")->load());
            phaser.setDepth(apvts.getRawParameterValue("phaserDepth")->load()/100);
            phaser.setCentreFrequency(apvts.getRawParameterValue("phaserFrequency")->load());
            phaser.setFeedback(apvts.getRawParameterValue("phaserFeedback")->load()/100);
        }

        void parameterChanged(const juce::String &parameterID, float newValue) override
        {
            updatePhaserParameters();
        }
        
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
    private:
        Phaser phaser;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaserUnit)
    };
}