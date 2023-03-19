/*
==============================================================================

    FXTremolo.h
    Created: 19 Mar 2023 7:18:47pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorBase.h"
#include "FXTremoloParameters.h"

using DryWetMixer = juce::dsp::DryWetMixer<float>;

class FXTremolo : public FXProcessorBase
{
public:
    FXTremolo()
    {
        dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
    }

    ~FXTremolo() {}

    juce::AudioProcessorEditor* createEditor() override { return nullptr; } // todo return the Compressor's editor object

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
        processSpec.numChannels = getNumOutputChannels();
        processSpec.sampleRate = sampleRate;
        dryWetMixer.prepare(processSpec);

        updateTremoloParameters();
    }

    void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> context(audioBlock);

        dryWetMixer.pushDrySamples(audioBlock);

        float amplitudeMultiplier = 0;

        auto *leftBufferPointer = buffer.getWritePointer(0);
        auto *rightBufferPointer = buffer.getWritePointer(1);

        for (size_t sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            amplitudeMultiplier = sin(currentAngle);
            amplitudeMultiplier = ( ( -1 * depth * amplitudeMultiplier ) + ( ( -1 * depth ) + 2 ) ) / 2;
            
            leftBufferPointer[sample] *= amplitudeMultiplier;

            if(isAutoPan)
            {
                amplitudeMultiplier = sin(currentAngle + juce::MathConstants<float>::pi);
                amplitudeMultiplier = ( ( -1 * depth * amplitudeMultiplier ) + ( ( -1 * depth ) + 2 ) ) / 2;
            }

            rightBufferPointer[sample] *= amplitudeMultiplier;

            currentAngle += angleDelta;
        }

        dryWetMixer.mixWetSamples(audioBlock);

        updateAngles();
    }

    void updateTremoloParameters()
    {
        dryWetMixer.setWetMixProportion(tremoloParameters.mix->get()/100);
        depth = tremoloParameters.depth->get()/100;
        rate = tremoloParameters.rate->get();
        isAutoPan = tremoloParameters.isAutoPan->get();
        updateAngles();
    }

    void registerListeners(juce::AudioProcessorValueTreeState &apvts)
    {
        apvts.addParameterListener("tremoloMix", &tremoloParameters);
        apvts.addParameterListener("tremoloDepth", &tremoloParameters);
        apvts.addParameterListener("tremoloRate", &tremoloParameters);
        apvts.addParameterListener("tremoloAutoPan", &tremoloParameters);
    }

    FXTremoloParameters tremoloParameters = { [this] () { updateTremoloParameters(); } };
private:
    DryWetMixer dryWetMixer;

    float depth = 0;
    float rate = 0;
    bool isAutoPan = false;

    float currentAngle = 0;
    float angleDelta = 0;

    void updateAngles()
    {
        auto sampleRate = getSampleRate();
        float cyclesPerSample = rate/sampleRate;
        angleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;
    }
};