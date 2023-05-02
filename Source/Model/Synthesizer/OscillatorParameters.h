/*
==============================================================================

    OscillatorParameters.h
    Created: 5 Apr 2023 8:36:19am
    Author:  habzd

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../Utils/WorkerThread.h"
#include "../../Utils/TripleBuffer.h"

namespace Processor::Synthesizer
{
    constexpr int HARMONIC_N = 256;                         //The number of harmonics the oscillator uses
    constexpr int LOOKUP_POINTS = HARMONIC_N * 32;          //The number of calculated points in the lookup table
    const int LOOKUP_SIZE = ceil(log2(HARMONIC_N) + 1);     //The number of mipmaps that need to be generated to avoid aliasing at a given harmonic count

    struct OscillatorParameters : public juce::AudioProcessorValueTreeState::Listener,
                                  public juce::Timer
    {
    public:
        OscillatorParameters(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
        {
            for(int i = 0; i < LOOKUP_SIZE; i++)
            {
                mipMap.add(std::make_unique<Utils::TripleBuffer<juce::dsp::LookupTableTransform<float>>>());
                auto& mipMapInstance = mipMap[i]->write();
                mipMapInstance.initialise([] (float x) { return 0; },
                        0,
                        juce::MathConstants<float>::twoPi,
                        2);
                mipMap[i]->release();
            }

            registerListener(this);
            linkParameters();

            startTimer(25);
        }

        ~OscillatorParameters() override
        {
            removeListener(this);
            lutUpdater.stopThread(10);
        }

        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const
        {
            auto paramLayoutSchema = createParameterLayout();

            auto params = paramLayoutSchema->getParameters(false);
            for ( auto param : params )
            {
                auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
                apvts.addParameterListener(id, listener);
            }
        }

        void removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const
        {
            auto paramLayoutSchema = createParameterLayout();
            auto params = paramLayoutSchema->getParameters(false);

            for(auto param : params)
            {
                auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
                apvts.removeParameterListener(id, listener);
            }
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

            for(int i = 0; i < HARMONIC_N; i++)
            {
                juce::String namePrefix = "Partial " + juce::String(i + 1) + " ";

                // Generating parameters to represent the amplitude percentage values of the partials
                auto partialGain = std::make_unique<juce::AudioParameterFloat>(
                    getPartialGainParameterID(i),
                    namePrefix + "Gain",
                    juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
                    0.f);
                oscGroup.get()->addChild(std::move(partialGain));

                // Generating parameters to represent the phase of the partials. These are represented as a percentage value of 2 * pi radians 
                auto partialPhase = std::make_unique<juce::AudioParameterFloat>(
                    getPartialPhaseParameterID(i),
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
        static const juce::String getPartialGainParameterID(size_t index)
        {
            return "partial" + juce::String(index) + "gain";
        }

        /// @brief Used for making the parameter ids of the the partials' phase parameters consistent
        /// @param index The index of the harmonic
        /// @return A parameter id
        static const juce::String getPartialPhaseParameterID(size_t index)
        {
            return "partial" + juce::String(index) + "phase";
        }

        /// @brief Generates a sample of the waveform defined by the parameters of the oscillator.
        /// @param angle The angle at which the sample is generated (in radians)
        /// @param harmonics The number of harmonics that are included in the calculation of the sample. Use lower numbers to avoid aliasing
        /// @return The generated sample
        const float getSample(float angle, int harmonics) const
        {
            jassert(harmonics <= HARMONIC_N);

            float sample = 0.f;

            for(int i = 0; i < harmonics; i++)
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

        /// @brief Calculates the peak amplitude of the waveform stored in the lookup table
        /// @return The peak amplitude
        const float getPeakAmplitude() const
        {
            float peakAmplitude = 0.f;
            float gainToNormalize = 1.f;
            for(int i = 0; i < LOOKUP_POINTS; i++)  //Finding the peak amplitude of the lut
            {
                float sample = getSample( juce::jmap( (float)i, 0.f, LOOKUP_POINTS-1.f, 0.f, juce::MathConstants<float>::twoPi), HARMONIC_N);

                if( std::fabs(sample) > peakAmplitude)
                {
                    peakAmplitude = std::fabs(sample);
                }
            }

            return peakAmplitude;
        }

        const juce::OwnedArray<Utils::TripleBuffer<juce::dsp::LookupTableTransform<float>>>& getLookupTable() const
        {
            return mipMap;
        }

        std::array<const std::atomic<float>*, HARMONIC_N> partialGains;
        std::array<const std::atomic<float>*, HARMONIC_N> partialPhases;
    private:
        juce::AudioProcessorValueTreeState& apvts;
        std::unordered_map<juce::String, std::atomic<float>> paramMap;

        juce::OwnedArray<Utils::TripleBuffer<juce::dsp::LookupTableTransform<float>>> mipMap;
        Utils::WorkerThread lutUpdater { [&] () { updateLookupTable(); } };
        std::atomic<bool> needUpdate = { false };

        void parameterChanged(const juce::String &parameterID, float newValue) override
        {
            paramMap[parameterID] = newValue;
            needUpdate = true;
        }

        void timerCallback()
        {
            if (!lutUpdater.isThreadRunning() && needUpdate)
            {
                needUpdate = false;
                lutUpdater.startThread();
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

            for(int i = 0; i < HARMONIC_N; i++)
            {
                partialGains[i] = &paramMap[getPartialGainParameterID(i)];
                partialPhases[i] = &paramMap[getPartialPhaseParameterID(i)];
            }
        }

        /// @brief Generates the lookup table with the current parameters
        void updateLookupTable()
        {
            float peakAmplitude = getPeakAmplitude();

            float gainToNormalize;
            for(int i = 0; i < LOOKUP_SIZE; i++)    //Generating peak-normalized lookup table
            {
                if(peakAmplitude > 0.f)
                {
                    gainToNormalize = 1.f / peakAmplitude;
                    auto& localMipMap = mipMap[i]->write();
                    localMipMap.initialise(
                        [this, i, gainToNormalize] (float x) { return gainToNormalize * getSample( x, std::floor( HARMONIC_N / pow(2, i) ) ); },
                        0,
                        juce::MathConstants<float>::twoPi,
                        LOOKUP_POINTS / pow(2, i));
                    mipMap[i]->release();
                }
                else
                {
                    auto& localMipMap = mipMap[i]->write();
                    localMipMap.initialise(
                        [] (float x) { return 0; },
                        0,
                        juce::MathConstants<float>::twoPi,
                        2);
                    mipMap[i]->release();
                }
            }
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorParameters)
    };
}