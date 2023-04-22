/*
==============================================================================

    FilterUnit.cpp
    Created: 11 Apr 2023 9:39:02pm
    Author:  Habama10

==============================================================================
*/

#include "FilterUnit.h"

#include "../../../View/Effects/EffectEditors/Filter/FilterEditor.h"

namespace Effects::Filter
{
    FilterUnit::FilterUnit(juce::AudioProcessorValueTreeState& apvts) : EffectProcessorUnit(apvts)
    {
        filters.add(std::make_unique<PassFilter>());
        filters.add(std::make_unique<PassFilter>());
        dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
        registerListener(this);
    }

    FilterUnit::~FilterUnit() 
    {
        removeListener(this);
    }

    void FilterUnit::prepareToPlay(double sampleRate, int samplesPerBlock) 
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

    void FilterUnit::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) 
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
    
    void FilterUnit::releaseResources() 
    {
        dryWetMixer.reset();
        for(auto filter : filters)
        {
            filter->reset();
        }
    }

    void FilterUnit::registerListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void FilterUnit::removeListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void FilterUnit::updateFilterParameters()
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
                updatePassFilter(*filter, coeffs, slope);
            }
        }
    }

    void FilterUnit::parameterChanged(const juce::String &parameterID, float newValue) 
    {
        updateFilterParameters();
    }

    juce::ReferenceCountedArray<Coefficients> FilterUnit::makeLowPassCoefficients(float frequency, int slope)
    {
        return FilterDesign::designIIRLowpassHighOrderButterworthMethod(frequency, getSampleRate(), slope+1);
    }

    juce::ReferenceCountedArray<Coefficients> FilterUnit::makeHighPassCoefficients(float frequency, int slope)
    {
        return FilterDesign::designIIRHighpassHighOrderButterworthMethod(frequency, getSampleRate(), slope+1);
    }

    void FilterUnit::updatePassFilter(PassFilter &chain, const juce::ReferenceCountedArray<Coefficients> &coefficients, const FilterSlope &slope)
    {
        chain.template setBypassed<0>(true);
        chain.template setBypassed<1>(true);
        
        switch(slope)
        {
            case s24dBOct:
            {
                update<1>(chain, coefficients);
            }
            case s18dBOct:
            {
                update<1>(chain, coefficients);
            }
            case s12dBOct:
            {
                update<0>(chain, coefficients);
            }
            case s6dBOct:
            {
                update<0>(chain, coefficients);
            }
        }
    }

    template<int Index>
    void FilterUnit::update(PassFilter& chain, const juce::ReferenceCountedArray<Coefficients> &coefficients)
    {
        updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
        chain.template setBypassed<Index>(false);
    }

    void FilterUnit::updateCoefficients(juce::ReferenceCountedObjectPtr<Coefficients> &oldCoefficients, const juce::ReferenceCountedObjectPtr<Coefficients> &newCoefficients)
    {
        *oldCoefficients = *newCoefficients;
    }

    EffectEditorUnit* FilterUnit::createEditorUnit()
    {
        return new FilterEditor(apvts);
    }
}