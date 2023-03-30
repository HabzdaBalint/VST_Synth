/*
==============================================================================

    FXCompressor.h
    Created: 14 Mar 2023 6:12:03pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorUnit.h"

using Compressor = juce::dsp::Compressor<float>;
using DryWetMixer = juce::dsp::DryWetMixer<float>;

class FXCompressor : public FXProcessorUnit
{
public:
    FXCompressor()
    {
        dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
    }

    ~FXCompressor() {}

    juce::AudioProcessorEditor* createEditor() override { return nullptr; } // todo return the Compressor's editor object

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        updateCompressorParameters();

        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
        processSpec.numChannels = getNumOutputChannels();
        processSpec.sampleRate = sampleRate;
        dryWetMixer.prepare(processSpec);
        compressor.prepare(processSpec);
    }

    void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        
        juce::dsp::ProcessContextReplacing<float> context(audioBlock);

        dryWetMixer.pushDrySamples(audioBlock);
        compressor.process(context);
        dryWetMixer.mixWetSamples(audioBlock);
    }

    void updateCompressorParameters()
    {
        dryWetMixer.setWetMixProportion(apvts->getRawParameterValue("compressorMix")->load()/100);
        compressor.setThreshold(apvts->getRawParameterValue("compressorThreshold")->load());
        compressor.setRatio(apvts->getRawParameterValue("compressorRatio")->load());
        compressor.setAttack(apvts->getRawParameterValue("compressorAttack")->load());
        compressor.setRelease(apvts->getRawParameterValue("compressorRelease")->load());
    }

    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout() override
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> compressorGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("compressorGroup", "Compressor", "|"));

        auto dryWetMix = std::make_unique<juce::AudioParameterFloat>("compressorMix",
                                                  "Wet%",
                                                  juce::NormalisableRange<float>(0.f, 100.f, 0.1), 100.f);
        compressorGroup.get()->addChild(std::move(dryWetMix));

        auto threshold = std::make_unique<juce::AudioParameterFloat>("compressorThreshold",
                                                  "Threshold",
                                                  juce::NormalisableRange<float>(-60.f, 0.f, 0.01), -18.f);
        compressorGroup.get()->addChild(std::move(threshold));

        auto ratio = std::make_unique<juce::AudioParameterFloat>("compressorRatio",
                                              "Ratio",
                                              juce::NormalisableRange<float>(1.f, 100.f, 0.01), 4.f);
        compressorGroup.get()->addChild(std::move(ratio));

        auto attack = std::make_unique<juce::AudioParameterFloat>("compressorAttack",
                                               "Attack",
                                               juce::NormalisableRange<float>(0.01, 250.f, 0.01), 0.5);
        compressorGroup.get()->addChild(std::move(attack));

        auto release = std::make_unique<juce::AudioParameterFloat>("compressorRelease",
                                                "Release",
                                                juce::NormalisableRange<float>(10.f, 1000.f, 0.1), 250.f);
        compressorGroup.get()->addChild(std::move(release));

        return compressorGroup;
    }
private:
    DryWetMixer dryWetMixer;
    Compressor compressor;

    void registerListeners() override
    {
        apvts->addParameterListener("compressorMix", this);
        apvts->addParameterListener("compressorThreshold", this);
        apvts->addParameterListener("compressorRatio", this);
        apvts->addParameterListener("compressorAttack", this);
        apvts->addParameterListener("compressorRelease", this);
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        triggerAsyncUpdate();
    }
    
    void handleAsyncUpdate() override 
    {
        updateCompressorParameters();
    }
};