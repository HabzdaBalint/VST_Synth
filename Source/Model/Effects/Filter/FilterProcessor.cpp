/*
==============================================================================

    FilterProcessor.cpp
    Created: 11 Apr 2023 9:39:02pm
    Author:  Habama10

==============================================================================
*/

#include "FilterProcessor.h"

#include "../../../View/Effects/Filter/FilterEditor.h"

namespace Effects::Filter
{
    FilterProcessor::FilterProcessor(juce::AudioProcessorValueTreeState& apvts) : EffectProcessor(apvts)
    {
        filters.add(std::make_unique<PassFilter>());
        filters.add(std::make_unique<PassFilter>());
        dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
        registerListener(this);
    }

    FilterProcessor::~FilterProcessor() 
    {
        removeListener(this);
    }

    void FilterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) 
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);
        
        juce::dsp::ProcessSpec filterSpec;
        filterSpec.maximumBlockSize = samplesPerBlock;
        filterSpec.numChannels = 1;
        filterSpec.sampleRate = sampleRate;

        for(auto& filter : filters)
        {
            filter->prepare(filterSpec);
        }

        juce::dsp::ProcessSpec dryWetSpec;
        dryWetSpec.maximumBlockSize = samplesPerBlock;
        dryWetSpec.numChannels = getTotalNumOutputChannels();
        dryWetSpec.sampleRate = sampleRate;
        dryWetMixer.prepare(dryWetSpec);
        updateFilterParameters();
    }

    void FilterProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) 
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);

        dryWetMixer.pushDrySamples(audioBlock);

        for(size_t channel = 0; channel < buffer.getNumChannels(); channel++)
        {
            auto singleBlock = audioBlock.getSingleChannelBlock(channel);
            juce::dsp::ProcessContextReplacing<float> singleContext(singleBlock);
            filters[channel]->process(singleContext);
        }

        dryWetMixer.mixWetSamples(audioBlock);
    }
    
    void FilterProcessor::releaseResources() 
    {
        dryWetMixer.reset();
        for(auto filter : filters)
        {
            filter->reset();
        }
    }

    void FilterProcessor::registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void FilterProcessor::removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void FilterProcessor::parameterChanged(const juce::String &parameterID, float newValue) 
    {
        updateFilterParameters();
    }
    
    void FilterProcessor::updateFilterParameters()
    {
        if(getSampleRate() > 0)
        {
            dryWetMixer.setWetMixProportion(apvts.getRawParameterValue("filterMix")->load()/100); 
            float frequency = apvts.getRawParameterValue("filterCutoff")->load();

            juce::ReferenceCountedArray<Coefficients> coeffs;

            FilterType type = static_cast<FilterType>(dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filterType"))->getIndex());
            FilterSlope slope = static_cast<FilterSlope>(dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filterSlope"))->getIndex());

            switch (type)
            {
                case lowPass:
                    coeffs = makeLowPassCoefficients(frequency, slope);
                    break;
                case highPass:
                    coeffs = makeHighPassCoefficients(frequency, slope);
                    break;
                default:
                    return;
            }

            for(auto filter : filters)
            {
                filter->setBypassed<0>(true);
                filter->setBypassed<1>(true);
                
                switch(slope)
                {
                    case s24dBOct:
                    {
                        updatePassFilter<1>(*filter, coeffs);
                        updatePassFilter<0>(*filter, coeffs);
                        break;
                    }
                    case s18dBOct:
                    {
                        updatePassFilter<1>(*filter, coeffs);
                        updatePassFilter<0>(*filter, coeffs);
                        break;
                    }
                    case s12dBOct:
                    {
                        updatePassFilter<0>(*filter, coeffs);
                        break;
                    }
                    case s6dBOct:
                    {
                        updatePassFilter<0>(*filter, coeffs);
                        break;
                    }
                    default:
                        return;
                }
            }
        }
    }

    juce::ReferenceCountedArray<Coefficients> FilterProcessor::makeLowPassCoefficients(float frequency, FilterSlope slope)
    {
        return FilterDesign::designIIRLowpassHighOrderButterworthMethod(frequency, getSampleRate(), slope+1);
    }

    juce::ReferenceCountedArray<Coefficients> FilterProcessor::makeHighPassCoefficients(float frequency, FilterSlope slope)
    {
        return FilterDesign::designIIRHighpassHighOrderButterworthMethod(frequency, getSampleRate(), slope+1);
    }

    template<int Index>
    void FilterProcessor::updatePassFilter(PassFilter& chain, const juce::ReferenceCountedArray<Coefficients>& coefficients)
    {
        *chain.get<Index>().coefficients = *coefficients[Index];
        chain.setBypassed<Index>(false);
    }

    EffectEditor* FilterProcessor::createEditorUnit()
    {
        return new FilterEditor(apvts);
    }
}