/*
==============================================================================

    Reverb.h
    Created: 14 Mar 2023 6:11:00pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorUnit.h"

namespace EffectProcessors
{
    using Reverb = juce::dsp::Reverb;

    class ReverbUnit : public FXProcessorUnit
    {
    public:
        ReverbUnit(juce::AudioProcessorValueTreeState& apvts) : FXProcessorUnit(apvts)
        {
            registerListeners();
        }

        ~ReverbUnit() {}

        void prepareToPlay(double sampleRate, int samplesPerBlock) override
        {
            setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);

            juce::dsp::ProcessSpec processSpec;
            processSpec.maximumBlockSize = samplesPerBlock;
            processSpec.numChannels = getTotalNumOutputChannels();
            processSpec.sampleRate = sampleRate;
            reverb.prepare(processSpec);
            updateReverbParameters();
        }

        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override
        {
            juce::dsp::AudioBlock<float> audioBlock(buffer);
            
            juce::dsp::ProcessContextReplacing<float> context(audioBlock);

            reverb.process(context);
        }

        void updateReverbParameters()
        {
            Reverb::Parameters newParams;
            newParams.wetLevel = apvts.getRawParameterValue("reverbWet")->load()/100;
            newParams.dryLevel = apvts.getRawParameterValue("reverbDry")->load()/100;
            newParams.roomSize = apvts.getRawParameterValue("reverbRoom")->load()/100;
            newParams.damping = apvts.getRawParameterValue("reverbDamping")->load()/100;
            newParams.width = apvts.getRawParameterValue("reverbWidth")->load()/100;
            reverb.setParameters(newParams);
        }

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
                35.f);
            reverbGroup.get()->addChild(std::move(wetLevel));

            auto dryLevel = std::make_unique<juce::AudioParameterFloat>(
                "reverbDry", 
                "Dry%",
                juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
                100.f);
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
    private:
        Reverb reverb;

        void registerListeners() override
        {
            apvts.addParameterListener("reverbWet", this);
            apvts.addParameterListener("reverbDry", this);
            apvts.addParameterListener("reverbRoom", this);
            apvts.addParameterListener("reverbDamping", this);
            apvts.addParameterListener("reverbWidth", this);
        }

        void parameterChanged(const juce::String &parameterID, float newValue) override
        {
            updateReverbParameters();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbUnit)
    };
}