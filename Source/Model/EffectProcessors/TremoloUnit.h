/*
==============================================================================

    Tremolo.h
    Created: 19 Mar 2023 7:18:47pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorUnit.h"

namespace EffectProcessors
{
    using DryWetMixer = juce::dsp::DryWetMixer<float>;

    class TremoloUnit : public FXProcessorUnit
    {
    public:
        TremoloUnit(juce::AudioProcessorValueTreeState& apvts) : FXProcessorUnit(apvts)
        {
            dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
            registerListener(this);
        }

        ~TremoloUnit() {}

        void prepareToPlay(double sampleRate, int samplesPerBlock) override
        {
            setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);

            juce::dsp::ProcessSpec processSpec;
            processSpec.maximumBlockSize = samplesPerBlock;
            processSpec.numChannels = getMainBusNumOutputChannels();
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

        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener)
        {
            auto paramLayoutSchema = createParameterLayout();
            auto params = paramLayoutSchema->getParameters(false);

            for(auto param : params)
            {
                auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
                apvts.addParameterListener(id, listener);
            }
        }

        void updateTremoloParameters()
        {
            dryWetMixer.setWetMixProportion(apvts.getRawParameterValue("tremoloMix")->load()/100);
            depth = apvts.getRawParameterValue("tremoloDepth")->load()/100;
            rate = apvts.getRawParameterValue("tremoloRate")->load();
            isAutoPan = apvts.getRawParameterValue("tremoloAutoPan")->load();
        }

        void parameterChanged(const juce::String &parameterID, float newValue) override
        {
            updateTremoloParameters();
        }

        static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
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

        void updateAngles()
        {
            auto sampleRate = getSampleRate();
            float cyclesPerSample = rate/sampleRate;
            angleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TremoloUnit)
    };
}

