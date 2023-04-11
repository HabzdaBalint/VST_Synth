/*
==============================================================================

    EqualizerUnit.h
    Created: 14 Mar 2023 6:11:56pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorUnit.h"

namespace EffectProcessors::Equalizer
{
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    constexpr int NUM_BANDS = 10;

    class EqualizerUnit : public FXProcessorUnit
    {
    public:
        EqualizerUnit(juce::AudioProcessorValueTreeState& apvts) : FXProcessorUnit(apvts)
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

        ~EqualizerUnit() override
        {
            removeListener(this);
        }

        void prepareToPlay(double sampleRate, int samplesPerBlock) override
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

        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override
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

        void releaseResources() override
        {
            for(auto equalizer : equalizers)
            {
                for(auto filter : *equalizer)
                {
                    filter->reset();
                }
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

        /// @brief Sets the new coefficients for the peak filters
        void updateEqualizerParameters()
        {
            if(getSampleRate() > 0)
            {
                for(auto equalizer : equalizers)
                {
                    updateEqualizerBands(*equalizer);
                }
            }
        }

        void parameterChanged(const juce::String &parameterID, float newValue) override
        {
            updateEqualizerParameters();
        }
        
        static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
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

        /// @brief Used for making the parameter ids of the the bands' gain parameters consistent
        /// @param index The index of the band
        /// @return A consistent parameter id
        static const juce::String getBandGainParameterName(size_t index)
        {
            return "band" + juce::String(index) + "gain";
        }

        /// @brief Used for getting usable frequency numbers from a bands' index
        /// @param index The index of the band
        /// @return A string containing the frequency the band is responsible for
        static const juce::String getBandFrequencyLabel(size_t index)
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

    private:
        juce::OwnedArray<juce::OwnedArray<Filter>> equalizers;

        void updateEqualizerBands(const juce::OwnedArray<Filter> &equalizer)
        {
            for (size_t i = 0; i < NUM_BANDS; i++)
            {
                float gain = apvts.getRawParameterValue(getBandGainParameterName(i))->load();
                updateBand(*equalizer[i], 31.25 * pow(2, i), proportionalQ(gain), juce::Decibels::decibelsToGain(gain));
            }
        }

        void updateBand(Filter& band, const float frequency, const float q, const float gain)
        {
            auto newCoeffs = Coefficients::makePeakFilter(getSampleRate(), frequency, q, gain);
            updateCoefficients(band.coefficients, newCoeffs);
        }
        
        void updateCoefficients(juce::ReferenceCountedObjectPtr<Coefficients> &oldCoefficients, const juce::ReferenceCountedObjectPtr<Coefficients> &newCoefficients)
        {
            *oldCoefficients = *newCoefficients;
        }

        /// @brief Scales the peak filter's Q to it's gain. Q starts at 0.5 at 0dB gain and goes up linearly to 3 at +/-12dB
        /// @param gain The gain level (dB) to use for scaling
        /// @return The proportional Q factor of the peak filter
        float proportionalQ(float gain)
        {
            float q = ((2.5 * std::abs(gain)) / 12) + 0.5;
            return q;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqualizerUnit)
    };
}