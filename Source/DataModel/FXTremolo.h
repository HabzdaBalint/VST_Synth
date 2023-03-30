/*
==============================================================================

    FXTremolo.h
    Created: 19 Mar 2023 7:18:47pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorUnit.h"

using DryWetMixer = juce::dsp::DryWetMixer<float>;

class FXTremolo : public FXProcessorUnit
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
        processSpec.numChannels = getTotalNumOutputChannels();
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

        updateAngles();

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
    }

    void updateTremoloParameters()
    {
        dryWetMixer.setWetMixProportion(apvts->getRawParameterValue("tremoloMix")->load()/100);
        depth = apvts->getRawParameterValue("tremoloDepth")->load()/100;
        rate = apvts->getRawParameterValue("tremoloRate")->load();
        isAutoPan = apvts->getRawParameterValue("tremoloAutoPan")->load();
    }

    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout() override
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> tremoloGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>(
                "tremoloGroup", 
                "Tremolo", 
                "|"));

        auto mix = std::make_unique<juce::AudioParameterFloat>(
            "tremoloMix", 
            "Wet%",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            100.f);
        tremoloGroup.get()->addChild(std::move(mix));

        auto depth = std::make_unique<juce::AudioParameterFloat>(
            "tremoloDepth", 
            "Depth",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            50.f);
        tremoloGroup.get()->addChild(std::move(depth));

        auto rate = std::make_unique<juce::AudioParameterFloat>(
            "tremoloRate", 
            "Rate",
            juce::NormalisableRange<float>(0.1, 15.f, 0.01, 0.35), 
            2.f);
        tremoloGroup.get()->addChild(std::move(rate));

        auto isAutoPan = std::make_unique<juce::AudioParameterBool>(
            "tremoloAutoPan", 
            "Auto-Pan",
            false);
        tremoloGroup.get()->addChild(std::move(isAutoPan));

        return tremoloGroup;
    }
    
private:
    DryWetMixer dryWetMixer;

    float depth = 0;
    float rate = 0;
    bool isAutoPan = false;

    float currentAngle = 0;
    float angleDelta = 0;

    void registerListeners() override
    {
        apvts->addParameterListener("tremoloMix", this);
        apvts->addParameterListener("tremoloDepth", this);
        apvts->addParameterListener("tremoloRate", this);
        apvts->addParameterListener("tremoloAutoPan", this);
    }

    void updateAngles()
    {
        auto sampleRate = getSampleRate();
        float cyclesPerSample = rate/sampleRate;
        angleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        updateTremoloParameters();
    }
};