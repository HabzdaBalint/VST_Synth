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

        ~OscillatorParameters() override
        {
            removeListener(this);
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

        void parameterChanged(const juce::String &parameterID, float newValue) override
        {
            paramMap[parameterID] = newValue;
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

            for (size_t i = 0; i < HARMONIC_N; i++)
            {
                juce::String namePrefix = "Partial " + juce::String(i + 1) + " ";

                // Generating parameters to represent the amplitude percentage values of the partials
                auto partialGain = std::make_unique<juce::AudioParameterFloat>(
                    getPartialGainParameterName(i),
                    namePrefix + "Gain",
                    juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
                    0.f);
                oscGroup.get()->addChild(std::move(partialGain));

                // Generating parameters to represent the phase of the partials. These are represented as a percentage value of 2 * pi radians 
                auto partialPhase = std::make_unique<juce::AudioParameterFloat>(
                    getPartialPhaseParameterName(i),
                    namePrefix + "Phase",
                    juce::NormalisableRange<float>(0.f, 99.9, 0.1), 
                    0.f);
                oscGroup.get()->addChild(std::move(partialPhase));
            }

            return oscGroup;
        }

        /// @brief Used for making the parameter ids of the the partials' gain parameters consistent
        /// @param index The index of the harmonic
        /// @return A parameter id
        static const juce::String getPartialGainParameterName(size_t index)
        {
            return "partial" + juce::String(index) + "gain";
        }

        /// @brief Used for making the parameter ids of the the partials' phase parameters consistent
        /// @param index The index of the harmonic
        /// @return A parameter id
        static const juce::String getPartialPhaseParameterName(size_t index)
        {
            return "partial" + juce::String(index) + "phase";
        }

        std::array<const std::atomic<float>*, HARMONIC_N> partialGains;
        std::array<const std::atomic<float>*, HARMONIC_N> partialPhases;
    private:
        juce::AudioProcessorValueTreeState& apvts;
        std::unordered_map<juce::String, std::atomic<float>> paramMap;

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

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorParameters)
    };
}
