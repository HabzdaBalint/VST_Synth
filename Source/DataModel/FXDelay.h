/*
==============================================================================

    FXDelay.h
    Created: 14 Mar 2023 6:12:18pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorBase.h"
#include "FXDelayParameters.h"

using Delay = juce::dsp::DelayLine<float>;
using Filter = juce::dsp::IIR::Filter<float>;
using Coefficients = juce::dsp::IIR::Coefficients<float>;

class FXDelay : public FXProcessorBase
{
public:
    FXDelay()
    {
        dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
    }

    ~FXDelay() {}

    juce::AudioProcessorEditor* createEditor() override { return nullptr; } // todo return the Compressor's editor object

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
        processSpec.numChannels = getNumOutputChannels();
        processSpec.sampleRate = sampleRate;
        dryWetMixer.prepare(processSpec);
        delay.prepare(processSpec);
        delay.setMaximumDelayInSamples( (int)std::round( ( DELAY_MAXLENGTH / 1000) * sampleRate ) );

        juce::dsp::ProcessSpec filterSpec;
        filterSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
        filterSpec.numChannels = 1;
        filterSpec.sampleRate = sampleRate;
        filter[0].prepare(filterSpec);
        filter[1].prepare(filterSpec);

        updateDelayParameters();
    }

    void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        
        juce::dsp::ProcessContextReplacing<float> context(audioBlock);

        dryWetMixer.pushDrySamples(audioBlock);

        for (size_t channel = 0; channel < 2; channel++)
        {
            auto *bufferPointer = buffer.getWritePointer(channel);

            for (size_t sample = 0; sample < buffer.getNumSamples(); sample++)
            {
                float delayedSample = delay.popSample(channel, delay.getDelay(), true);

                float filteredSample = filter[channel].processSample(delayedSample);

                delay.pushSample(channel, bufferPointer[sample] + filteredSample*feedback);

                bufferPointer[sample] = filteredSample;
            }
            filter[channel].snapToZero();
        }
        dryWetMixer.mixWetSamples(audioBlock);
    }

    void updateDelayParameters()
    {
        if(getSampleRate() > 0)
        {
            dryWetMixer.setWetMixProportion(delayParameters.mix->get()/100);
            feedback = delayParameters.feedback->get()/100;
            delay.setDelay( (float)std::round( ( delayParameters.time->get() / 1000 ) * getSampleRate() ) );
            float freq = delayParameters.filterFrequency->get();
            float q = delayParameters.filterQ->get();
            filter[0].coefficients = Coefficients::makeBandPass(getSampleRate(), freq, q);
            filter[1].coefficients = Coefficients::makeBandPass(getSampleRate(), freq, q);
        }
    }

    void registerListeners(juce::AudioProcessorValueTreeState &apvts)
    {
        apvts.addParameterListener("delayMix", &delayParameters);
        apvts.addParameterListener("delayFeedback", &delayParameters);
        apvts.addParameterListener("delayTime", &delayParameters);
        apvts.addParameterListener("delayFilterFrequency", &delayParameters);
        apvts.addParameterListener("delayFilterQ", &delayParameters);
    }

    FXDelayParameters delayParameters{ [this] () { updateDelayParameters(); } };
private:
    DryWetMixer dryWetMixer;
    Delay delay;
    Filter filter[2];

    float feedback = 0;
};