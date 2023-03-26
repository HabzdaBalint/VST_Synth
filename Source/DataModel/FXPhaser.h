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

    void connectApvts(juce::AudioProcessorValueTreeState& apvts)
    {
        this->apvts = &apvts;
        registerListeners();
    }

    void updatePhaserParameters()
    {
        phaser.setMix(apvts->getRawParameterValue("phaserMix")->load()/100);
        phaser.setRate(apvts->getRawParameterValue("phaserRate")->load());
        phaser.setDepth(apvts->getRawParameterValue("phaserDepth")->load()/100);
        phaser.setCentreFrequency(apvts->getRawParameterValue("phaserFrequency")->load());
        phaser.setFeedback(apvts->getRawParameterValue("phaserFeedback")->load()/100);
    }

    FXPhaserParameters phaserParameters = { [this] () { updatePhaserParameters(); } };
private:
    juce::AudioProcessorValueTreeState* apvts;
    juce::AudioProcessorValueTreeState localapvts = { *this, nullptr, "Phaser Parameters", phaserParameters.createParameterLayout() };;

    Phaser phaser;

    void registerListeners()
    {
        apvts->addParameterListener("phaserMix", &phaserParameters);
        apvts->addParameterListener("phaserRate", &phaserParameters);
        apvts->addParameterListener("phaserDepth", &phaserParameters);
        apvts->addParameterListener("phaserFrequency", &phaserParameters);
        apvts->addParameterListener("phaserFeedback", &phaserParameters);
    }
};