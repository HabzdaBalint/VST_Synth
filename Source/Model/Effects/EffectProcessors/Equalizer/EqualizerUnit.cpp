/*
==============================================================================

    EqualizerUnit.cpp
    Created: 11 Apr 2023 9:38:53pm
    Author:  Habama10

==============================================================================
*/

#include "EqualizerUnit.h"

#include "../../../../View/Effects/EffectEditors/Equalizer/EqualizerEditor.h"

namespace Effects::EffectProcessors::Equalizer
{
    EqualizerUnit::EqualizerUnit(juce::AudioProcessorValueTreeState& apvts) : EffectProcessorUnit(apvts)
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

    EqualizerUnit::~EqualizerUnit() 
    {
        removeListener(this);
    }

    void EqualizerUnit::prepareToPlay(double sampleRate, int samplesPerBlock) 
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

        updateEqualizerParameters();
    }

    void EqualizerUnit::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) 
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

    void EqualizerUnit::releaseResources() 
    {
        for(auto equalizer : equalizers)
        {
            for(auto filter : *equalizer)
            {
                filter->reset();
            }
        }
    }

    void EqualizerUnit::registerListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void EqualizerUnit::removeListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void EqualizerUnit::updateEqualizerParameters()
    {
        if(getSampleRate() > 0)
        {
            for(auto equalizer : equalizers)
            {
                updateEqualizerBands(*equalizer);
            }
        }
    }

    void EqualizerUnit::parameterChanged(const juce::String &parameterID, float newValue) 
    {
        updateEqualizerParameters();
    }
    
    std::unique_ptr<juce::AudioProcessorParameterGroup> EqualizerUnit::createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> eqGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>(
                "eqGroup", 
                "Equalizer", 
                "|"));

        for (size_t i = 0; i < NUM_BANDS; i++)
        {
            auto bandGain = std::make_unique<juce::AudioParameterFloat>(
                getBandGainParameterName(i),
                getBandFrequencyLabel(i),
                juce::NormalisableRange<float>(-12.f, 12.f, 0.1), 
                0.f);
            eqGroup.get()->addChild(std::move(bandGain));
        }
        
        return eqGroup;
    }

    const juce::String EqualizerUnit::getBandGainParameterName(size_t index)
    {
        return "band" + juce::String(index) + "gain";
    }

    const juce::String EqualizerUnit::getBandFrequencyLabel(size_t index)
    {
        float frequency = 31.25 * pow(2, index);
        juce::String suffix;
        if(frequency >= 1000)
        {
            suffix = " kHz";
            frequency /= 1000;
        }
        else
        {
            suffix = " Hz";
        }
        juce::String label(frequency, 0, false);
        return label + suffix;
    }

    void EqualizerUnit::updateEqualizerBands(const juce::OwnedArray<Filter> &equalizer)
    {
        for (size_t i = 0; i < NUM_BANDS; i++)
        {
            float gain = apvts.getRawParameterValue(getBandGainParameterName(i))->load();
            updateBand(*equalizer[i], 31.25 * pow(2, i), proportionalQ(gain, 0.25), juce::Decibels::decibelsToGain(gain));
        }
    }

    void EqualizerUnit::updateBand(Filter& band, const float frequency, const float q, const float gain)
    {
        auto newCoeffs = Coefficients::makePeakFilter(getSampleRate(), frequency, q, gain);
        updateCoefficients(band.coefficients, newCoeffs);
    }
    
    void EqualizerUnit::updateCoefficients(juce::ReferenceCountedObjectPtr<Coefficients> &oldCoefficients, const juce::ReferenceCountedObjectPtr<Coefficients> &newCoefficients)
    {
        *oldCoefficients = *newCoefficients;
    }

    float EqualizerUnit::proportionalQ(const float gain, const float constant)
    {
        float q = constant * std::abs(gain);
        return q;
    }

    EffectEditorUnit* EqualizerUnit::createEditorUnit()
    {
        return new EqualizerEditor(apvts);
    }
}