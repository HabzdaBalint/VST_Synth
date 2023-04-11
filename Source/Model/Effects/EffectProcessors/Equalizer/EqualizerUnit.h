/*
==============================================================================

    EqualizerUnit.h
    Created: 14 Mar 2023 6:11:56pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "../EffectProcessorUnit.h"

namespace Effects::EffectProcessors::Equalizer
{
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    constexpr int NUM_BANDS = 10;

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
        static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout();

        /// @brief Used for making the parameter ids of the the bands' gain parameters consistent
        /// @param index The index of the band
        /// @return A consistent parameter id
        static const juce::String getBandGainParameterName(size_t index);

        /// @brief Used for getting usable frequency numbers from a bands' index
        /// @param index The index of the band
        /// @return A string containing the frequency the band is responsible for
        static const juce::String getBandFrequencyLabel(size_t index);
        
        EffectEditorUnit* createEditorUnit() override;

    private:
        juce::OwnedArray<juce::OwnedArray<Filter>> equalizers;

        /// @brief Sets the new coefficients for the peak filters
        void updateEqualizerParameters();

        void updateEqualizerBands(const juce::OwnedArray<Filter> &equalizer);

        void updateBand(Filter& band, const float frequency, const float q, const float gain);
        
        void updateCoefficients(juce::ReferenceCountedObjectPtr<Coefficients> &oldCoefficients, const juce::ReferenceCountedObjectPtr<Coefficients> &newCoefficients);

        /// @brief Scales the peak filter's Q to it's gain. Q starts at 0.5 at 0dB gain and goes up linearly to 3 at +/-12dB
        /// @param gain The gain level (dB) to use for scaling
        /// @return The proportional Q factor of the peak filter
        float proportionalQ(float gain);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqualizerUnit)
    };
}