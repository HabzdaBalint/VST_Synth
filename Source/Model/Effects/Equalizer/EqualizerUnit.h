/*
==============================================================================

    EqualizerUnit.h
    Created: 14 Mar 2023 6:11:56pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessorUnit.h"

namespace Effects::Equalizer
{
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    constexpr int NUM_BANDS = 10;

    /// @brief Used for making the parameter ids of the the bands' gain parameters consistent
    /// @param index The index of the band
    /// @return A parameter id
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

    class EqualizerUnit : public EffectProcessorUnit
    {
    public:
        EqualizerUnit(juce::AudioProcessorValueTreeState& apvts);
        ~EqualizerUnit() override;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
        void releaseResources() override;

        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener);
        void removeListener(juce::AudioProcessorValueTreeState::Listener* listener);

        void parameterChanged(const juce::String &parameterID, float newValue) override;
        
        EffectEditorUnit* createEditorUnit() override;

    private:
        juce::OwnedArray<juce::OwnedArray<Filter>> equalizers;

        /// @brief Sets the new coefficients for the peak filters
        void updateEqualizerParameters();

        void updateEqualizerBands(const juce::OwnedArray<Filter> &equalizer);

        void updateBand(Filter& band, const float frequency, const float q, const float gain);
        
        void updateCoefficients(juce::ReferenceCountedObjectPtr<Coefficients> &oldCoefficients, const juce::ReferenceCountedObjectPtr<Coefficients> &newCoefficients);

        /// @brief Scales the peak filter's Q to it's gain.
        /// @param gain The gain level (dB) to use for scaling
        /// @param constant The value gain is multiplied by
        /// @return The proportional Q factor of the peak filter
        float proportionalQ(const float gain, const float constant);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqualizerUnit)
    };
}