/*
==============================================================================

    FilterProcessor.h
    Created: 14 Mar 2023 6:11:50pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessor.h"

namespace Processor::Effects::Filter
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
        Low_Pass,
        High_Pass
    };

    static const juce::StringArray filterTypeChoices = {"Low-pass", "High-pass"};
    static const juce::StringArray filterSlopeChoices = {"6dB/Oct", "12dB/Oct", "18dB/Oct", "24dB/Oct"};
    
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

    class FilterProcessor : public EffectProcessor
    {
    public:
        FilterProcessor(juce::AudioProcessorValueTreeState& apvts);
        ~FilterProcessor() override;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
        void releaseResources() override;

        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const;
        void removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const;

        Editor::Effects::EffectEditor* createEditorUnit() override;

        const juce::String getName() const override
        {
            return "Filter";
        }

    private:
        juce::AudioProcessorValueTreeState& apvts;

        juce::OwnedArray<PassFilter> filters;
        DryWetMixer dryWetMixer;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void updateFilterParameters();

        juce::ReferenceCountedArray<Coefficients> makeLowPassCoefficients(float frequency, FilterSlope slope);
        juce::ReferenceCountedArray<Coefficients> makeHighPassCoefficients(float frequency, FilterSlope slope);

        template<int Index>
        void updatePassFilter(PassFilter& filter, const juce::ReferenceCountedArray<Coefficients>& coefficients);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterProcessor);
    };
}