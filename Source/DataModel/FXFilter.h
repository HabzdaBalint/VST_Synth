/*
==============================================================================

    FXFilter.h
    Created: 14 Mar 2023 6:11:50pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorUnit.h"

using Filter = juce::dsp::IIR::Filter<float>;
using Coefficients = juce::dsp::IIR::Coefficients<float>;
using FilterDesign = juce::dsp::FilterDesign<float>;
using PassFilter = juce::dsp::ProcessorChain<Filter, Filter>;

enum FilterSlope
{
    s6dBOct,
    s12dBOct,
    s18dBOct,
    s24dBOct
};

enum FilterType
{
    lowPass,
    highPass
};

static const juce::StringArray typeChoices = {"Low-pass", "High-pass"};
static const juce::StringArray slopeChoices = {"6dB/Oct", "12dB/Oct", "18dB/Oct", "24dB/Oct"};

class FXFilter : public FXProcessorUnit
{
public:
    FXFilter(juce::AudioProcessorValueTreeState& apvts) : FXProcessorUnit(apvts)
    {
        dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
        registerListeners();
    }

    ~FXFilter(){}

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);
        
        juce::dsp::ProcessSpec filterSpec;
        filterSpec.maximumBlockSize = samplesPerBlock;
        filterSpec.numChannels = 1;
        filterSpec.sampleRate = sampleRate;
        leftChain.prepare(filterSpec);
        rightChain.prepare(filterSpec);

        juce::dsp::ProcessSpec dryWetSpec;
        dryWetSpec.maximumBlockSize = samplesPerBlock;
        dryWetSpec.numChannels = getTotalNumOutputChannels();
        dryWetSpec.sampleRate = sampleRate;
        dryWetMixer.prepare(dryWetSpec);
        updateFilterParameters();
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

    void updateFilterParameters()
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

            updatePassFilter(leftChain, coeffs, slope);
            updatePassFilter(rightChain, coeffs, slope);
        }
    }

    static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> filterGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("filterGroup", "Filter", "|"));

        auto dryWetMix = std::make_unique<juce::AudioParameterFloat>(
            "filterMix",
            "Wet%",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            100.f);
        filterGroup.get()->addChild(std::move(dryWetMix));
        
        auto filterType = std::make_unique<juce::AudioParameterChoice>(
            "filterType", 
            "Filter Type", 
            typeChoices, 
            0);
        filterGroup.get()->addChild(std::move(filterType));

        auto filterSlope = std::make_unique<juce::AudioParameterChoice>(
            "filterSlope",
            "Filter Slope", 
            slopeChoices, 
            0);
        filterGroup.get()->addChild(std::move(filterSlope));

        auto cutoffFrequency = std::make_unique<juce::AudioParameterFloat>(
            "filterCutoff",
            "Cutoff Frequency",
            juce::NormalisableRange<float>(10.f, 22000.f, 0.1, 0.25), 
            500.f);
        filterGroup.get()->addChild(std::move(cutoffFrequency));

        return filterGroup;
    }

private:
    PassFilter leftChain;
    PassFilter rightChain;
    juce::dsp::DryWetMixer<float> dryWetMixer;

    void registerListeners() override
    {
        apvts.addParameterListener("filterMix", this);
        apvts.addParameterListener("filterCutoff", this);
        apvts.addParameterListener("filterType", this);
        apvts.addParameterListener("filterSlope", this);
    }

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

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        updateFilterParameters();
    }
};