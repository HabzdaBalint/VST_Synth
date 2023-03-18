/*
==============================================================================

    FXPhaser.h
    Created: 14 Mar 2023 6:11:14pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorBase.h"
#include "FXPhaserParameters.h"

using Phaser = juce::dsp::Phaser<float>;

class FXPhaser : public FXProcessorBase
{
public:
    FXPhaser() {}

    ~FXPhaser() {}

    juce::AudioProcessorEditor* createEditor() override { return nullptr; } // todo return the Phaser's editor object

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
        processSpec.numChannels = getNumOutputChannels();
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

    void updatePhaserParameters()
    {
        phaser.setMix(phaserParameters.mix->get()/100);
        phaser.setRate(phaserParameters.rate->get());
        phaser.setDepth(phaserParameters.depth->get()/100);
        phaser.setCentreFrequency(phaserParameters.frequency->get());
        phaser.setFeedback(phaserParameters.feedback->get()/100);
    }

    void registerListeners(juce::AudioProcessorValueTreeState &apvts)
    {
        apvts.addParameterListener("phaserMix", &phaserParameters);
        apvts.addParameterListener("phaserRate", &phaserParameters);
        apvts.addParameterListener("phaserDepth", &phaserParameters);
        apvts.addParameterListener("phaserFrequency", &phaserParameters);
        apvts.addParameterListener("phaserFeedback", &phaserParameters);
    }

    FXPhaserParameters phaserParameters = { [this] () { updatePhaserParameters(); } };
private:
    Phaser phaser;
};