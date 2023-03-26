/*
==============================================================================

    FXEqualizer.h
    Created: 14 Mar 2023 6:11:56pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorBase.h"
#include "FXEqualizerParameters.h"

using Filter = juce::dsp::IIR::Filter<float>;
using Coefficients = juce::dsp::IIR::Coefficients<float>;

class FXEqualizer : public FXProcessorBase
{
public:
    FXEqualizer()
    {
        for (size_t i = 0; i < 10; i++)
        {
            leftFilters[i] = new Filter();
            rightFilters[i] = new Filter();
        }
    }

    ~FXEqualizer()
    {
        for (size_t i = 0; i < 10; i++)
        {
            delete(leftFilters[i]);
            delete(rightFilters[i]);
        }
    }

    juce::AudioProcessorEditor* createEditor() override { return nullptr; } // todo return the EQ's editor object

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        updateEqualizerParameters();
        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
        processSpec.numChannels = 1;
        processSpec.sampleRate = sampleRate;

        for (size_t i = 0; i < 10; i++)
        {
            leftFilters[i]->prepare(processSpec);
            rightFilters[i]->prepare(processSpec);
        }
    }

    void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        auto leftBlock = audioBlock.getSingleChannelBlock(0);
        auto rightBlock = audioBlock.getSingleChannelBlock(1);
        
        juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
        juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

        for (size_t i = 0; i < 10; i++)
        {
            leftFilters[i]->process(leftContext);
            rightFilters[i]->process(rightContext);
        }
    }

    void connectApvts(juce::AudioProcessorValueTreeState& apvts)
    {
        this->apvts = &apvts;
        registerListeners();
    }

    /// @brief Sets the new coefficients for the peak filters
    void updateEqualizerParameters()
    {
        if(getSampleRate() > 0)
        {            
            for (int i = 0; i < 10; i++)
            {
                float gain = apvts->getRawParameterValue(equalizerParameters.getBandGainParameterName(i))->load();
                leftFilters[i]->coefficients = Coefficients::makePeakFilter(getSampleRate(), 31.25 * pow(2, i), proportionalQ(gain), juce::Decibels::decibelsToGain(gain));
                rightFilters[i]->coefficients = Coefficients::makePeakFilter(getSampleRate(), 31.25 * pow(2, i), proportionalQ(gain), juce::Decibels::decibelsToGain(gain));
            }
        }
    }

    FXEqualizerParameters equalizerParameters{ [this] () { updateEqualizerParameters(); } };
private:
    juce::AudioProcessorValueTreeState* apvts;
    juce::AudioProcessorValueTreeState localapvts = { *this, nullptr, "Equalizer Parameters", equalizerParameters.createParameterLayout() };;

    Filter* leftFilters[10] = {};
    Filter* rightFilters[10] = {};

    void registerListeners()
    {
        for (size_t i = 0; i < 10; i++)
        {
            apvts->addParameterListener(equalizerParameters.getBandGainParameterName(i), &equalizerParameters);
        }
    }

    /// @brief Scales the peak filter's Q to it's gain. Q starts at 1 at 0dB gain and goes up linearly to 4 at +/-12dB
    /// @param gain The gain level (in dB) to use for scaling
    /// @return The proportional Q factor of the peak filter
    float proportionalQ(float gain)
    {
        float q = ((3 * std::abs(gain)) / 12) + 1;
        return q;
    }
};