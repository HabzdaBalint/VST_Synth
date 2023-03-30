/*
==============================================================================

    FXReverb.h
    Created: 14 Mar 2023 6:11:00pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorUnit.h"

using Reverb = juce::dsp::Reverb;

class FXReverb : public FXProcessorUnit
{
public:
    FXReverb() {}

    ~FXReverb() {}

    juce::AudioProcessorEditor* createEditor() override { return nullptr; } // todo return the Reverb's editor object

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
        processSpec.numChannels = getNumOutputChannels();
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
        newParams.wetLevel = apvts->getRawParameterValue("reverbWet")->load()/100;
        newParams.dryLevel = apvts->getRawParameterValue("reverbDry")->load()/100;
        newParams.roomSize = apvts->getRawParameterValue("reverbRoom")->load()/100;
        newParams.damping = apvts->getRawParameterValue("reverbDamping")->load()/100;
        newParams.width = apvts->getRawParameterValue("reverbWidth")->load()/100;
        reverb.setParameters(newParams);
    }

    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout() override
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> reverbGroup (
                    std::make_unique<juce::AudioProcessorParameterGroup>("reverbGroup", "Reverb", "|"));
        auto wetLevel = std::make_unique<juce::AudioParameterFloat>("reverbWet", 
                                                 "Wet%",
                                                 juce::NormalisableRange<float>(0.f, 100.f, 0.1), 35.f);
        reverbGroup.get()->addChild(std::move(wetLevel));

        auto dryLevel = std::make_unique<juce::AudioParameterFloat>("reverbDry", 
                                                 "Dry%",
                                                 juce::NormalisableRange<float>(0.f, 100.f, 0.1), 100.f);
        reverbGroup.get()->addChild(std::move(dryLevel));

        auto roomSize = std::make_unique<juce::AudioParameterFloat>("reverbRoom", 
                                                 "Room Size",
                                                 juce::NormalisableRange<float>(0.f, 100.f, 0.1), 50.f);
        reverbGroup.get()->addChild(std::move(roomSize));

        auto damping = std::make_unique<juce::AudioParameterFloat>("reverbDamping", 
                                                "Damping",
                                                juce::NormalisableRange<float>(0.f, 100.f, 0.1), 50.f);
        reverbGroup.get()->addChild(std::move(damping));

        auto width = std::make_unique<juce::AudioParameterFloat>("reverbWidth", 
                                              "Width",
                                              juce::NormalisableRange<float>(0.f, 100.f, 0.1), 50.f);
        reverbGroup.get()->addChild(std::move(width));

        return reverbGroup;
    }    
private:
    Reverb reverb;

    void registerListeners() override
    {
        apvts->addParameterListener("reverbWet", this);
        apvts->addParameterListener("reverbDry", this);
        apvts->addParameterListener("reverbRoom", this);
        apvts->addParameterListener("reverbDamping", this);
        apvts->addParameterListener("reverbWidth", this);
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        triggerAsyncUpdate();
    }
    
    void handleAsyncUpdate() override 
    {
        updateReverbParameters();
    }
};