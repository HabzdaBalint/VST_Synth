/*
==============================================================================

    EqualizerProcessor.cpp
    Created: 11 Apr 2023 9:38:53pm
    Author:  Habama10

==============================================================================
*/

#include "EqualizerProcessor.h"

#include "../../../View/Effects/Equalizer/EqualizerEditor.h"

namespace Effects::Equalizer
{
    EqualizerProcessor::EqualizerProcessor(juce::AudioProcessorValueTreeState& apvts) : EffectProcessor(apvts)
    {
        for (size_t i = 0; i < 2; i++)
        {
            equalizers.add(std::make_unique<juce::OwnedArray<Filter>>());
            for (size_t j = 0; j < NUM_BANDS; j++)
            {
                equalizers[i]->add(std::make_unique<Filter>());
            }
        }
        
        registerListener(this);
    }

    EqualizerProcessor::~EqualizerProcessor() 
    {
        removeListener(this);
    }

    void EqualizerProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) 
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);
        
        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = samplesPerBlock;
        processSpec.numChannels = 1;
        processSpec.sampleRate = sampleRate;

        for(auto equalizer : equalizers)
        {
            for(auto filter : *equalizer)
            {
                filter->prepare(processSpec);
            }
        }

        for(auto equalizer : equalizers)
        {
            for (size_t i = 0; i < NUM_BANDS; i++)
            {
                float gain = apvts.getRawParameterValue(getBandGainParameterID(i))->load();
                updateBand(*(*equalizer)[i], getFrequency(i), proportionalQ(gain, Q_SCALE), juce::Decibels::decibelsToGain(gain));
            }
        }
    }

    void EqualizerProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) 
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);

        for(size_t channel = 0; channel < buffer.getNumChannels(); channel++)
        {
            auto singleBlock = audioBlock.getSingleChannelBlock(channel);
            juce::dsp::ProcessContextReplacing<float> singleContext(singleBlock);

            for(auto filter : *equalizers[channel])
            {
                filter->process(singleContext);
            }
        }
    }

    void EqualizerProcessor::releaseResources() 
    {
        for(auto equalizer : equalizers)
        {
            for(auto filter : *equalizer)
            {
                filter->reset();
            }
        }
    }

    void EqualizerProcessor::registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void EqualizerProcessor::removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void EqualizerProcessor::parameterChanged(const juce::String &parameterID, float newValue) 
    {
        int index = getBandIndexFromParameterID(parameterID);
        for(auto equalizer : equalizers)
        {
            float gain = apvts.getRawParameterValue(getBandGainParameterID(index))->load();

            updateBand(*(*equalizer)[index], getFrequency(index), proportionalQ(gain, Q_SCALE), juce::Decibels::decibelsToGain(gain));
        }
    }

    void EqualizerProcessor::updateBand(Filter& band, const float frequency, const float q, const float gain)
    {
        auto newCoeffs = Coefficients::makePeakFilter(getSampleRate(), frequency, q, gain);
        *band.coefficients = *newCoeffs;
    }

    const float EqualizerProcessor::proportionalQ(const float gain, const float constant) const
    {
        float q = constant * std::abs(gain);
        return (q == 0.f) ? 1.f : q;
    }

    const float EqualizerProcessor::getFrequency(const int index) const
    {
        return 31.25 * pow(2, index);
    }

    EffectEditor* EqualizerProcessor::createEditorUnit()
    {
        return new EqualizerEditor(apvts);
    }
}