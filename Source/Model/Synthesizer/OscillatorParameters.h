/*
==============================================================================

    OscillatorParameters.h
    Created: 5 Apr 2023 8:36:19am
    Author:  habzd

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Synthesizer
{
    constexpr int HARMONIC_N = 256;             //The number of harmonics the oscillator uses

    struct OscillatorParameters : public juce::AudioProcessorValueTreeState::Listener
    {
    public:
        OscillatorParameters(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
        {
            registerListener(this);
            linkParameters();
        }

        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener)
        {
            auto paramLayoutSchema = createParameterLayout();

            auto params = paramLayoutSchema->getParameters(false);
            for ( auto param : params )
            {
                auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
                apvts.addParameterListener(id, listener);
            }
        }

        void linkParameters()
        {
            auto paramLayoutSchema = createParameterLayout();

            auto params = paramLayoutSchema->getParameters(false);
            for ( auto param : params )
            {
                auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
                auto value = dynamic_cast<juce::RangedAudioParameter*>(param)->getNormalisableRange().convertFrom0to1(param->getDefaultValue());
                paramMap.emplace(id, value);
            }

            for (size_t i = 0; i < HARMONIC_N; i++)
            {
                partialGains[i] = &paramMap[getPartialGainParameterName(i)];
                partialPhases[i] = &paramMap[getPartialPhaseParameterName(i)];
            }
        }

        void parameterChanged(const juce::String &parameterID, float newValue) override
        {
            paramMap[parameterID] = newValue;
        }

        /// @brief Generates a sample of the waveform defined by the parameters of the synthesizer. Used for maintaining the lookup table. This function could also be used for accurate rendering, with any number of harmonics, if time is not a constraint
        /// @param angle The angle at which the sample is generated (in radians)
        /// @param harmonics The number of harmonics that are included in the calculation of the sample. Use lower numbers to avoid aliasing
        /// @return The generated sample
        const float getSample(float angle, int harmonics)
        {
            jassert(harmonics <= HARMONIC_N);

            float sample = 0.f;

            for (size_t i = 0; i < harmonics; i++)
            {
                float gain = partialGains[i]->load() / 100;
                float phase = partialPhases[i]->load() / 100;
                if (gain != 0.f)
                {
                    sample += gain * sin((i + 1) * angle + phase * juce::MathConstants<float>::twoPi);
                }
            }

            return sample;
        }

        /// @brief Creates and adds the synthesizer's parameters into a parameter group
        /// @return unique pointer to the group
        static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
        {
            std::unique_ptr<juce::AudioProcessorParameterGroup> oscGroup (
                std::make_unique<juce::AudioProcessorParameterGroup>(
                    "oscGroup", 
                    "Oscillator", 
                    "|"));

            juce::AudioParameterFloatAttributes attr;

            for (size_t i = 0; i < HARMONIC_N; i++)
            {
                juce::String namePrefix = "Partial " + juce::String(i + 1) + " ";

                // Generating parameters to represent the amplitude percentage values of the partials
                auto partialGain = std::make_unique<juce::AudioParameterFloat>(
                    getPartialGainParameterName(i),
                    namePrefix + "Gain",
                    juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
                    0.f,
                    attr.withAutomatable(false).withMeta(true));
                oscGroup.get()->addChild(std::move(partialGain));

                // Generating parameters to represent the phase of the partials. These are represented as a percentage value of 2 * pi radians 
                auto partialPhase = std::make_unique<juce::AudioParameterFloat>(
                    getPartialPhaseParameterName(i),
                    namePrefix + "Phase",
                    juce::NormalisableRange<float>(0.f, 99.9, 0.1), 
                    0.f,
                    attr.withAutomatable(false).withMeta(true));
                oscGroup.get()->addChild(std::move(partialPhase));
            }

            return oscGroup;
        }

        /// @brief Used for making the parameter ids of the the partials' gain parameters consistent
        /// @param index The index of the harmonic
        /// @return A consistent parameter id
        static const juce::String getPartialGainParameterName(size_t index)
        {
            return "partial" + juce::String(index) + "gain";
        }

        /// @brief Used for making the parameter ids of the the partials' phase parameters consistent
        /// @param index The index of the harmonic
        /// @return A consistent parameter id
        static const juce::String getPartialPhaseParameterName(size_t index)
        {
            return "partial" + juce::String(index) + "phase";
        }

        const std::atomic<float>* partialGains[HARMONIC_N];
        const std::atomic<float>* partialPhases[HARMONIC_N];
    private:
        juce::AudioProcessorValueTreeState& apvts;
        std::unordered_map<juce::String, std::atomic<float>> paramMap;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorParameters)
    };
}
