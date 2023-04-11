/*
==============================================================================

    FilterUnit.h
    Created: 14 Mar 2023 6:11:50pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorUnit.h"

namespace EffectProcessors::Filter
{
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    using FilterDesign = juce::dsp::FilterDesign<float>;
    using PassFilter = juce::dsp::ProcessorChain<Filter, Filter>;
    using DryWetMixer = juce::dsp::DryWetMixer<float>;

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

    static const juce::StringArray filterTypeChoices = {"Low-pass", "High-pass"};
    static const juce::StringArray filterSlopeChoices = {"6dB/Oct", "12dB/Oct", "18dB/Oct", "24dB/Oct"};

    class FilterUnit : public FXProcessorUnit
    {
    public:
        FilterUnit(juce::AudioProcessorValueTreeState& apvts) : FXProcessorUnit(apvts)
        {
            filters.add(std::make_unique<PassFilter>());
            filters.add(std::make_unique<PassFilter>());
            dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
            registerListener(this);
        }

        ~FilterUnit() override
        {
            removeListener(this);
        }

        void prepareToPlay(double sampleRate, int samplesPerBlock) override
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

        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override
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
        
        void releaseResources() override
        {
            dryWetMixer.reset();
            for(auto filter : filters)
            {
                filter->reset();
            }
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

        void removeListener(juce::AudioProcessorValueTreeState::Listener* listener)
        {
            auto paramLayoutSchema = createParameterLayout();
            auto params = paramLayoutSchema->getParameters(false);

            for(auto param : params)
            {
                auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
                apvts.removeParameterListener(id, listener);
            }
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

                for(auto filter : filters)
                {
                    updatePassFilter(*filter, coeffs, slope);
                }
            }
        }

        void parameterChanged(const juce::String &parameterID, float newValue) override
        {
            updateFilterParameters();
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
                filterTypeChoices, 
                0);
            filterGroup.get()->addChild(std::move(filterType));

            auto filterSlope = std::make_unique<juce::AudioParameterChoice>(
                "filterSlope",
                "Filter Slope", 
                filterSlopeChoices, 
                0);
            filterGroup.get()->addChild(std::move(filterSlope));

            auto cutoffFrequency = std::make_unique<juce::AudioParameterFloat>(
                "filterCutoff",
                "Cutoff Frequency",
                juce::NormalisableRange<float>(10.f, 22000.f, 0.1, 0.25), 
                1000.f);
            filterGroup.get()->addChild(std::move(cutoffFrequency));

            return filterGroup;
        }

    private:
        juce::OwnedArray<PassFilter> filters;
        DryWetMixer dryWetMixer;

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

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterUnit);
    };
}