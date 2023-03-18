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

    void updateChorusParameters()
    {
        chorus.setMix(chorusParameters.mix->get()/100);
        chorus.setRate(chorusParameters.rate->get());
        chorus.setCentreDelay(chorusParameters.delay->get());
        chorus.setDepth(chorusParameters.depth->get()/100);
        chorus.setFeedback(chorusParameters.feedback->get()/100);
    }

    void registerListeners(juce::AudioProcessorValueTreeState &apvts)
    {
        apvts.addParameterListener("chorusMix", &chorusParameters);
        apvts.addParameterListener("chorusRate", &chorusParameters);
        apvts.addParameterListener("chorusDelay", &chorusParameters);
        apvts.addParameterListener("chorusDepth", &chorusParameters);
        apvts.addParameterListener("chorusFeedback", &chorusParameters);
    }

    FXChorusParameters chorusParameters = { [this] () { updateChorusParameters(); } };
private:
    Chorus chorus;
};