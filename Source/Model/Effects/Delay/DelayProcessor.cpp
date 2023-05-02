/*
==============================================================================

    DelayProcessor.cpp
    Created: 11 Apr 2023 9:38:39pm
    Author:  Habama10

==============================================================================
*/

#include "DelayProcessor.h"

#include "../../../View/Effects/Delay/DelayEditor.h"

namespace Processor::Effects::Delay
{
    DelayProcessor::DelayProcessor(juce::AudioProcessorValueTreeState& apvts) : EffectProcessor(apvts)
    {
        filters.add(std::make_unique<Filter>());
        filters.add(std::make_unique<Filter>());
        dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
        registerListener(this);
    }

    DelayProcessor::~DelayProcessor()
    {
        removeListener(this);
    }

    void DelayProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) 
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

    void DelayProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) 
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        
        juce::dsp::ProcessContextReplacing<float> context(audioBlock);

        dryWetMixer.pushDrySamples(audioBlock);

        for(int channel = 0; channel < 2; channel++)
        {
            auto *bufferPointer = buffer.getWritePointer(channel);

            for(int sample = 0; sample < buffer.getNumSamples(); sample++)
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

    void DelayProcessor::releaseResources() 
    {
        dryWetMixer.reset();
        delay.reset();
    }

    void DelayProcessor::registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void DelayProcessor::removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void DelayProcessor::updateDelayParameters()
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

    void DelayProcessor::parameterChanged(const juce::String &parameterID, float newValue) 
    {
        updateDelayParameters();
    }
    
    Editor::Effects::EffectEditor* DelayProcessor::createEditorUnit()
    {
        return new Editor::Effects::DelayEditor(apvts);
    }
}