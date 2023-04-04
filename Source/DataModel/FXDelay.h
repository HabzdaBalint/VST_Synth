/*
==============================================================================

    FXDelay.h
    Created: 14 Mar 2023 6:12:18pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorUnit.h"

using Delay = juce::dsp::DelayLine<float>;
using Filter = juce::dsp::IIR::Filter<float>;
using Coefficients = juce::dsp::IIR::Coefficients<float>;
using DryWetMixer = juce::dsp::DryWetMixer<float>;

constexpr float DELAY_MAXLENGTH = 1000.f;

class FXDelay : public FXProcessorUnit
{
public:
    FXDelay(juce::AudioProcessorValueTreeState& apvts) : FXProcessorUnit(apvts)
    {
        dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
        registerListeners();
    }

    ~FXDelay() {}

    juce::AudioProcessorEditor* createEditor() override { return nullptr; } // todo return the Compressor's editor object

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
        processSpec.numChannels = getTotalNumOutputChannels();
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
            dryWetMixer.setWetMixProportion(apvts.getRawParameterValue("delayMix")->load()/100);
            feedback = apvts.getRawParameterValue("delayFeedback")->load()/100;
            delay.setDelay( (float)std::round( ( apvts.getRawParameterValue("delayTime")->load() / 1000 ) * getSampleRate() ) );
            float freq = apvts.getRawParameterValue("delayFilterFrequency")->load();
            float q = apvts.getRawParameterValue("delayFilterQ")->load();

            filter[0].coefficients = Coefficients::makeBandPass(getSampleRate(), freq, q);
            filter[1].coefficients = Coefficients::makeBandPass(getSampleRate(), freq, q);
        }
    }

    static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> delayGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>(
                "delayGroup", 
                "Delay", 
                "|"));

        auto mix = std::make_unique<juce::AudioParameterFloat>(
            "delayMix",
            "Wet%",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            35.f);                                
        delayGroup.get()->addChild(std::move(mix));

        auto feedback = std::make_unique<juce::AudioParameterFloat>(
            "delayFeedback",
            "Feedback",
            juce::NormalisableRange<float>(0.f, 100.f, 1.f), 
            40.f);
        delayGroup.get()->addChild(std::move(feedback));

        auto time = std::make_unique<juce::AudioParameterFloat>(
            "delayTime",
            "Time",
            juce::NormalisableRange<float>(1.f, DELAY_MAXLENGTH, 0.1, 0.5),
            250.f);
        delayGroup.get()->addChild(std::move(time));
        
        auto filterFrequency = std::make_unique<juce::AudioParameterFloat>(
            "delayFilterFrequency",
            "Center Frequency",
            juce::NormalisableRange<float>(10.f, 22000.f, 0.1, 0.25),
            500.f);
        delayGroup.get()->addChild(std::move(filterFrequency));
        
        auto filterQ = std::make_unique<juce::AudioParameterFloat>(
            "delayFilterQ",
            "Q",
            juce::NormalisableRange<float>(0.05, 5.f, 0.001),
            0.5);
        delayGroup.get()->addChild(std::move(filterQ));
        
        return delayGroup;
    }

private:
    DryWetMixer dryWetMixer;
    Delay delay;
    Filter filter[2];

    float feedback = 0;

    void registerListeners() override
    {
        apvts.addParameterListener("delayMix", this);
        apvts.addParameterListener("delayFeedback", this);
        apvts.addParameterListener("delayTime", this);
        apvts.addParameterListener("delayFilterFrequency", this);
        apvts.addParameterListener("delayFilterQ", this);
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        updateDelayParameters();
    }
};