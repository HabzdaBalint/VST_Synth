/*
==============================================================================

    FXReverb.h
    Created: 14 Mar 2023 6:11:00pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorBase.h"
#include "FXReverbParameters.h"

using Reverb = juce::dsp::Reverb;

class FXReverb : public FXProcessorBase
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

    void connectApvts(juce::AudioProcessorValueTreeState& apvts)
    {
        this->apvts = &apvts;
        registerListeners();
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

    FXReverbParameters reverbParameters = { [this] () { updateReverbParameters(); } };
private:
    juce::AudioProcessorValueTreeState* apvts;
    juce::AudioProcessorValueTreeState localapvts = { *this, nullptr, "Reverb Parameters", reverbParameters.createParameterLayout() };;

    Reverb reverb;

    void registerListeners()
    {
        apvts->addParameterListener("reverbWet", &reverbParameters);
        apvts->addParameterListener("reverbDry", &reverbParameters);
        apvts->addParameterListener("reverbRoom", &reverbParameters);
        apvts->addParameterListener("reverbDamping", &reverbParameters);
        apvts->addParameterListener("reverbWidth", &reverbParameters);
    }
};