/*
==============================================================================

    FXFilter.h
    Created: 14 Mar 2023 6:11:50pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorBase.h"
#include "FXFilterParameters.h"

using Filter = juce::dsp::IIR::Filter<float>;
using Coefficients = juce::dsp::IIR::Coefficients<float>;
using FilterDesign = juce::dsp::FilterDesign<float>;
using PassFilter = juce::dsp::ProcessorChain<Filter, Filter>;

class FXFilter : public FXProcessorBase
{
public:
    FXFilter()
    {
        dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
    }

    ~FXFilter(){}

    juce::AudioProcessorEditor* createEditor() override { return nullptr; } // todo return the Filter's editor object

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        updateFilterParameters();
        juce::dsp::ProcessSpec filterSpec;
        filterSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
        filterSpec.numChannels = 1;
        filterSpec.sampleRate = sampleRate;
        leftChain.prepare(filterSpec);
        rightChain.prepare(filterSpec);

        juce::dsp::ProcessSpec dryWetSpec;
        dryWetSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
        dryWetSpec.numChannels = getNumOutputChannels();
        dryWetSpec.sampleRate = sampleRate;
        dryWetMixer.prepare(dryWetSpec);
    }

    void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        auto leftBlock = audioBlock.getSingleChannelBlock(0);
        auto rightBlock = audioBlock.getSingleChannelBlock(1);
        
        juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
        juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

        dryWetMixer.pushDrySamples(audioBlock);
        leftChain.process(leftContext);
        rightChain.process(rightContext);
        dryWetMixer.mixWetSamples(audioBlock);
    }

    /// @brief Sets the new coefficients for the peak filters
    void updateFilterParameters()
    {
        if(getSampleRate() > 0)
        {
            dryWetMixer.setWetMixProportion(filterParameters.dryWetMix->get()/100); 
            float frequency = filterParameters.cutoffFrequency->get();
            juce::ReferenceCountedArray<Coefficients> coeffs;
            FilterSlope slope = static_cast<FilterSlope>(filterParameters.filterSlope->getIndex());
            FilterType type = static_cast<FilterType>(filterParameters.filterType->getIndex());

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

            updatePassFilter(leftChain, coeffs, slope);
            updatePassFilter(rightChain, coeffs, slope);
        }
    }

    void registerListeners(juce::AudioProcessorValueTreeState &apvts)
    {
        apvts.addParameterListener("filterMix", &filterParameters);
        apvts.addParameterListener("filterCutoff", &filterParameters);
        apvts.addParameterListener("filterType", &filterParameters);
        apvts.addParameterListener("filterSlope", &filterParameters);
    }

    FXFilterParameters filterParameters { [this] () { updateFilterParameters(); } };
private:
    PassFilter leftChain;
    PassFilter rightChain;
    juce::dsp::DryWetMixer<float> dryWetMixer;

    juce::ReferenceCountedArray<Coefficients> makeLowPassCoefficients(float frequency, int slope)
    {
        return FilterDesign::designIIRLowpassHighOrderButterworthMethod(frequency, getSampleRate(), slope+1);
    }

    juce::ReferenceCountedArray<Coefficients> makeHighPassCoefficients(float frequency, int slope)
    {
        return FilterDesign::designIIRHighpassHighOrderButterworthMethod(frequency, getSampleRate(), slope+1);
    }

    void updatePassFilter(PassFilter &chain, const juce::ReferenceCountedArray<Coefficients> &coefficients, const FilterSlope &slope)
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
    void update(PassFilter& chain, const juce::ReferenceCountedArray<Coefficients> &coefficients)
    {
        updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
        chain.template setBypassed<Index>(false);
    }

    void updateCoefficients(juce::ReferenceCountedObjectPtr<Coefficients> &oldCoefficients, const juce::ReferenceCountedObjectPtr<Coefficients> &newCoefficients)
    {
        *oldCoefficients = *newCoefficients;
    }
};