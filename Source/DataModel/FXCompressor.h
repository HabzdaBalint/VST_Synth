/*
==============================================================================

    FXCompressor.h
    Created: 14 Mar 2023 6:12:03pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorBase.h"
#include "FXCompressorParameters.h"

using Compressor = juce::dsp::Compressor<float>;
using DryWetMixer = juce::dsp::DryWetMixer<float>;

class FXCompressor : public FXProcessorBase
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

    void connectApvts(juce::AudioProcessorValueTreeState& apvts)
    {
        this->apvts = &apvts;
        registerListeners();
    }

    void updateCompressorParameters()
    {
        dryWetMixer.setWetMixProportion(apvts->getRawParameterValue("compressorMix")->load()/100);
        compressor.setThreshold(apvts->getRawParameterValue("compressorThreshold")->load());
        compressor.setRatio(apvts->getRawParameterValue("compressorRatio")->load());
        compressor.setAttack(apvts->getRawParameterValue("compressorAttack")->load());
        compressor.setRelease(apvts->getRawParameterValue("compressorRelease")->load());
    }

    FXCompressorParameters compressorParameters{ [this] () { updateCompressorParameters(); } };
private:
    juce::AudioProcessorValueTreeState* apvts;
    juce::AudioProcessorValueTreeState localapvts = { *this, nullptr, "Compressor Parameters", compressorParameters.createParameterLayout() };;

    DryWetMixer dryWetMixer;
    Compressor compressor;

    void registerListeners()
    {
        apvts->addParameterListener("compressorMix", &compressorParameters);
        apvts->addParameterListener("compressorThreshold", &compressorParameters);
        apvts->addParameterListener("compressorRatio", &compressorParameters);
        apvts->addParameterListener("compressorAttack", &compressorParameters);
        apvts->addParameterListener("compressorRelease", &compressorParameters);
    }
};