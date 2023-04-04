/*
==============================================================================

    FXPhaser.h
    Created: 14 Mar 2023 6:11:14pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorUnit.h"

using Phaser = juce::dsp::Phaser<float>;

class FXPhaser : public FXProcessorUnit
{
public:
    FXPhaser(juce::AudioProcessorValueTreeState& apvts) : FXProcessorUnit(apvts)
    {
        registerListeners();
    }

    ~FXPhaser() {}

    juce::AudioProcessorEditor* createEditor() override { return nullptr; } // todo return the Phaser's editor object

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
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

    void updatePhaserParameters()
    {
        phaser.setMix(apvts.getRawParameterValue("phaserMix")->load()/100);
        phaser.setRate(apvts.getRawParameterValue("phaserRate")->load());
        phaser.setDepth(apvts.getRawParameterValue("phaserDepth")->load()/100);
        phaser.setCentreFrequency(apvts.getRawParameterValue("phaserFrequency")->load());
        phaser.setFeedback(apvts.getRawParameterValue("phaserFeedback")->load()/100);
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
            50.f);
        phaserGroup.get()->addChild(std::move(depth));

        auto frequency = std::make_unique<juce::AudioParameterFloat>(
            "phaserFrequency", 
            "Frequency",
            juce::NormalisableRange<float>(10.f, 22000.f, 0.1, 0.25),
            500.f);
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

    void registerListeners() override
    {
        apvts.addParameterListener("phaserMix", this);
        apvts.addParameterListener("phaserRate", this);
        apvts.addParameterListener("phaserDepth", this);
        apvts.addParameterListener("phaserFrequency", this);
        apvts.addParameterListener("phaserFeedback", this);
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        updatePhaserParameters();
    }
};