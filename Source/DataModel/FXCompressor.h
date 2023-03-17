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

    /// @brief Sets the new coefficients for the peak filters
    void updateCompressorParameters()
    {
        if(getSampleRate() > 0)
        {
            dryWetMixer.setWetMixProportion(compressorParameters.dryWetMix->get()/100);
            compressor.setThreshold(compressorParameters.threshold->get());
            compressor.setRatio(compressorParameters.ratio->get());
            compressor.setAttack(compressorParameters.attack->get());
            compressor.setRelease(compressorParameters.release->get());
        }
    }

    void registerListeners(juce::AudioProcessorValueTreeState &apvts)
    {
        apvts.addParameterListener("compressorMix", &compressorParameters);
        apvts.addParameterListener("compressorThreshold", &compressorParameters);
        apvts.addParameterListener("compressorRatio", &compressorParameters);
        apvts.addParameterListener("compressorAttack", &compressorParameters);
        apvts.addParameterListener("compressorRelease", &compressorParameters);
    }

    FXCompressorParameters compressorParameters{ [this] () { updateCompressorParameters(); } };
private:
    DryWetMixer dryWetMixer;
    Compressor compressor;
};