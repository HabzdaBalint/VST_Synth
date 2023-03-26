/*
==============================================================================

    FXChorus.h
    Created: 14 Mar 2023 6:11:06pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorBase.h"
#include "FXChorusParameters.h"

using Chorus = juce::dsp::Chorus<float>;

class FXChorus : public FXProcessorBase
{
public:
    FXChorus() {}

    ~FXChorus() {}

    juce::AudioProcessorEditor* createEditor() override { return nullptr; } // todo return the Chorus's editor object

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
        processSpec.numChannels = getNumOutputChannels();
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

    void connectApvts(juce::AudioProcessorValueTreeState& apvts)
    {
        this->apvts = &apvts;
        registerListeners();
    }

    void updateChorusParameters()
    {
        chorus.setMix(apvts->getRawParameterValue("chorusMix")->load()/100);
        chorus.setMix(apvts->getRawParameterValue("chorusRate")->load());
        chorus.setMix(apvts->getRawParameterValue("chorusDelay")->load());
        chorus.setMix(apvts->getRawParameterValue("chorusDepth")->load()/100);
        chorus.setMix(apvts->getRawParameterValue("chorusFeedback")->load()/100);
    }

    FXChorusParameters chorusParameters = { [this] () { updateChorusParameters(); } };
private:
    juce::AudioProcessorValueTreeState* apvts;
    juce::AudioProcessorValueTreeState localapvts = { *this, nullptr, "Chorus Parameters", chorusParameters.createParameterLayout() };;

    Chorus chorus;
    
    void registerListeners()
    {
        apvts->addParameterListener("chorusMix", &chorusParameters);
        apvts->addParameterListener("chorusRate", &chorusParameters);
        apvts->addParameterListener("chorusDelay", &chorusParameters);
        apvts->addParameterListener("chorusDepth", &chorusParameters);
        apvts->addParameterListener("chorusFeedback", &chorusParameters);
    }
};