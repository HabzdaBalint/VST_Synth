/*
==============================================================================

    ChorusUnit.h
    Created: 14 Mar 2023 6:11:06pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorUnit.h"

namespace EffectProcessors
{
    using Chorus = juce::dsp::Chorus<float>;

    class ChorusUnit : public FXProcessorUnit
    {
    public:
        ChorusUnit(juce::AudioProcessorValueTreeState& apvts) : FXProcessorUnit(apvts)
        {
            registerListeners();
        }

        ~ChorusUnit() {}

        void prepareToPlay(double sampleRate, int samplesPerBlock) override
        {
            setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);

            juce::dsp::ProcessSpec processSpec;
            processSpec.maximumBlockSize = samplesPerBlock;
            processSpec.numChannels = getTotalNumOutputChannels();
            processSpec.sampleRate = sampleRate;
            chorus.prepare(processSpec);
            updateChorusParameters();
        }

        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override
        {
            juce::dsp::AudioBlock<float> audioBlock(buffer);
            
            juce::dsp::ProcessContextReplacing<float> context(audioBlock);

            chorus.process(context);
        }

        void updateChorusParameters()
        {
            chorus.setMix(apvts.getRawParameterValue("chorusMix")->load()/100);
            chorus.setRate(apvts.getRawParameterValue("chorusRate")->load());
            chorus.setCentreDelay(apvts.getRawParameterValue("chorusDelay")->load());
            chorus.setDepth(apvts.getRawParameterValue("chorusDepth")->load()/100);
            chorus.setFeedback(apvts.getRawParameterValue("chorusFeedback")->load()/100);
        }

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
                2.f);
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
                50.f);
            chorusGroup.get()->addChild(std::move(depth));

            auto feedback = std::make_unique<juce::AudioParameterFloat>(
                "chorusFeedback", 
                "Feedback",
                juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
                40.f);
            chorusGroup.get()->addChild(std::move(feedback));

            return chorusGroup;
        }

    private:
        Chorus chorus;
        
        void registerListeners() override
        {
            apvts.addParameterListener("chorusMix", this);
            apvts.addParameterListener("chorusRate", this);
            apvts.addParameterListener("chorusDelay", this);
            apvts.addParameterListener("chorusDepth", this);
            apvts.addParameterListener("chorusFeedback", this);
        }

        void parameterChanged(const juce::String &parameterID, float newValue) override
        {
            updateChorusParameters();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChorusUnit)
    };
}