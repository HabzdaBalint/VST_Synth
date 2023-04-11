/*
==============================================================================

    DelayUnit.h
    Created: 14 Mar 2023 6:12:18pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorUnit.h"

namespace EffectProcessors::Delay
{
    using Delay = juce::dsp::DelayLine<float>;
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    using DryWetMixer = juce::dsp::DryWetMixer<float>;

    constexpr float MAX_LENGTH_MS = 1000.f;

    class DelayUnit : public FXProcessorUnit
    {
    public:
        DelayUnit(juce::AudioProcessorValueTreeState& apvts) : FXProcessorUnit(apvts)
        {
            filters.add(std::make_unique<Filter>());
            filters.add(std::make_unique<Filter>());
            dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
            registerListener(this);
        }

        ~DelayUnit() {}

        void prepareToPlay(double sampleRate, int samplesPerBlock) override
        {
            setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);

            juce::dsp::ProcessSpec processSpec;
            processSpec.maximumBlockSize = samplesPerBlock;
            processSpec.numChannels = getTotalNumOutputChannels();
            processSpec.sampleRate = sampleRate;
            dryWetMixer.prepare(processSpec);
            delay.prepare(processSpec);
            delay.setMaximumDelayInSamples( int(MAX_LENGTH_MS / 1000.f) * sampleRate );

            juce::dsp::ProcessSpec filterSpec;
            filterSpec.maximumBlockSize = samplesPerBlock;
            filterSpec.numChannels = 1;
            filterSpec.sampleRate = sampleRate;
            
            for(auto& filter : filters)
            {
                filter->prepare(filterSpec);
            }

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

                    float filteredSample = filters[channel]->processSample(delayedSample);

                    delay.pushSample(channel, bufferPointer[sample] + filteredSample*feedback);

                    bufferPointer[sample] = filteredSample;
                }
                filters[channel]->snapToZero();
            }
            dryWetMixer.mixWetSamples(audioBlock);
        }

        void releaseResources() override
        {
            dryWetMixer.reset();
            delay.reset();
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

        void updateDelayParameters()
        {
            if(getSampleRate() > 0)
            {
                dryWetMixer.setWetMixProportion(apvts.getRawParameterValue("delayMix")->load()/100);
                feedback = apvts.getRawParameterValue("delayFeedback")->load()/100;
                delay.setDelay( (float)std::round( ( apvts.getRawParameterValue("delayTime")->load() / 1000 ) * getSampleRate() ) );
                float freq = apvts.getRawParameterValue("delayFilterFrequency")->load();
                float q = apvts.getRawParameterValue("delayFilterQ")->load();

                for(auto& filter : filters)
                {
                    filter->coefficients = Coefficients::makeBandPass(getSampleRate(), freq, q);
                }
            }
        }

        void parameterChanged(const juce::String &parameterID, float newValue) override
        {
            updateDelayParameters();
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
                juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
                40.f);
            delayGroup.get()->addChild(std::move(feedback));

            auto time = std::make_unique<juce::AudioParameterFloat>(
                "delayTime",
                "Time",
                juce::NormalisableRange<float>(1.f, MAX_LENGTH_MS, 0.1, 0.5),
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
                juce::NormalisableRange<float>(0.05, 5.f, 0.001, 0.4),
                0.5);
            delayGroup.get()->addChild(std::move(filterQ));
            
            return delayGroup;
        }

    private:
        DryWetMixer dryWetMixer;
        Delay delay;
        juce::OwnedArray<Filter> filters;

        float feedback = 0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayUnit)
    };
}