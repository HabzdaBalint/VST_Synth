/*
==============================================================================

    AdditiveVoice.h
    Created: 11 Mar 2023 2:17:04am
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "OscillatorParameters.h"
#include "AdditiveSynthParameters.h"

namespace Processor::Synthesizer
{
    struct UnisonPairAngleData
    {
        float upperCurrentAngle[2] = {0.f , 0.f};
        float lowerCurrentAngle[2] = {0.f , 0.f};
        float upperAngleDelta = 0.f;
        float lowerAngleDelta = 0.f;
        float upperFrequencyOffset = 0.f;
        float lowerFrequencyOffset = 0.f;

        void reset()
        {
            upperCurrentAngle[0] = 0.f;
            upperCurrentAngle[1] = 0.f;
            lowerCurrentAngle[0] = 0.f;
            lowerCurrentAngle[1] = 0.f;
            upperAngleDelta = 0.f;
            lowerAngleDelta = 0.f;
            upperFrequencyOffset = 0.f;
            lowerFrequencyOffset = 0.f;
        }
    };

    struct VoiceAngleData
    {
        float currentAngle[2] = {0.f , 0.f};
        float angleDelta = 0.f;
        float frequency = 0.f;

        UnisonPairAngleData unisonData[5];

        void reset()
        {
            currentAngle[0] = 0.f;
            currentAngle[1] = 0.f;
            angleDelta = 0.f;
            frequency = 0.f;

            for (int i = 0; i < 5; i++)
            {
                unisonData[i].reset();
            }
        }
    };

    class AdditiveVoice : public juce::SynthesiserVoice
    {
    public:
        AdditiveVoice(AdditiveSynthParameters& synthParams, const juce::OwnedArray<Utils::TripleBuffer<juce::dsp::LookupTableTransform<float>>>& mipMap);

        bool canPlaySound(juce::SynthesiserSound* sound) override { return sound != nullptr; }
        void controllerMoved(int controllerNumber, int newControllerValue) override {}
        bool isVoiceActive() const override;

        void pitchWheelMoved(int newPitchWheelValue) override;
        void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
        void stopNote(float velocity, bool allowTailOff) override;

        void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    private:
        juce::AudioBuffer<float> generatedBuffer;
        AdditiveSynthParameters& synthParameters;
        
        juce::Random rng;

        const juce::OwnedArray<Utils::TripleBuffer<juce::dsp::LookupTableTransform<float>>>& mipMap;

        VoiceAngleData voiceData;

        float velocityGain = 0;
        int currentNote = 0;
        bool bypassPlaying = false;

        int unisonPairCount = 0;
        float unisonGain = 0.f;

        float pitchWheelOffset = 0;

        float highestCurrentFrequency = 0.f;
        int mipMapIndex = 0;

        juce::ADSR amplitudeADSR;

        /// @brief Generates a sample for the fundamental of the voice
        /// @param channel The channel where the sample is needed (for random phase per channel)
        /// @return The generated sample
        const float getFundamentalSample(const int channel, const juce::dsp::LookupTableTransform<float>& localMipMap);

        /// @brief Generates a sample for the unison of the voice
        /// @param channel The channel where the sample is needed (for random phase per channel)
        /// @param unisonNumber The unison pair to generate with
        /// @return The generated sample
        const float getUnisonSample(const int channel, const int unisonNumber, const juce::dsp::LookupTableTransform<float>& localMipMap);

        /// @brief Used to randomise the starting phases of all generated waveforms
        void updatePhases();

        /// @brief Used to generate a random phase offset based on the configured starting position and range
        /// @return Returns the generated offset
        const float getRandomPhase();

        /// @brief Called to update frequencies with current parameters
        void updateFrequencies();

        /// @brief Used to generate new angle deltas for the given frequencies the voice is expected to generate
        void updateAngles();

        /// @brief Checks the highest possible overtone the current highest generated frequency (including the up-tuned unison voices) that can safely be generated without aliasing at the current sample-rate and selects the right lookup table with the correct number of overtones. Playback is skipped entirely if such a look-up table doesn't exist
        void findMipMapToUse();

        /// @brief Used to stop playback and reset values when a note off message arrives
        void resetProperties();

        void updateADSRParams();
    };
}
