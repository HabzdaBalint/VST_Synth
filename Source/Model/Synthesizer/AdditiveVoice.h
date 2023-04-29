/*
==============================================================================

    AdditiveVoice.h
    Created: 11 Mar 2023 2:17:04am
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "AdditiveSynthParameters.h"

namespace Synthesizer
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

            for (size_t i = 0; i < 5; i++)
            {
                unisonData[i].reset();
            }
        }
    };

    class AdditiveVoice : public juce::SynthesiserVoice
    {
    public:
        AdditiveVoice(AdditiveSynthParameters& synthParams, const juce::OwnedArray<juce::dsp::LookupTableTransform<float>>& mipMap) :
            synthParameters(synthParams),
            mipMap(mipMap) {}

        ~AdditiveVoice() {}

        bool canPlaySound(juce::SynthesiserSound* sound) override { return sound != nullptr; }

        void controllerMoved(int controllerNumber, int newControllerValue) override {}

        bool isVoiceActive() const override
        {
            return ( getCurrentlyPlayingNote() >= 0 || amplitudeADSR.isActive() );
        }

        void pitchWheelMoved(int newPitchWheelValue) override
        {
            pitchWheelOffset = ((float)newPitchWheelValue-8192)/8192;

            updateFrequencies();
            updateAngles();
        }

        void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override
        {
            currentNote = midiNoteNumber;
            velocityGain = velocity;
            pitchWheelOffset = ((float)currentPitchWheelPosition-8192)/8192;

            updatePhases();
            updateFrequencies();
            updateAngles();

            updateADSRParams();
            if(amplitudeADSR.isActive())
                amplitudeADSR.reset();
            amplitudeADSR.noteOn();
        }

        void stopNote(float velocity, bool allowTailOff) override
        {
            if( !isSustainPedalDown() && !isSostenutoPedalDown() )
                amplitudeADSR.noteOff();
            
            if( !allowTailOff || !amplitudeADSR.isActive() )
            {
                clearCurrentNote();
                resetProperties();
            }
        }

        /// @brief Audio callback function. Generates audio buffer
        /// @param outputBuffer The incoming (and outgoing) buffer where the generated data needs to go
        /// @param startSample The starting sample within the buffer
        /// @param numSamples Length of the buffer
        void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
        {   //No point in updating variables and calculating samples if velocity is 0 or if the voice is not in use
            if( isVoiceActive() && velocityGain > 0.f )
            {
                if( !bypassPlaying )
                {
                    generatedBuffer.clear();
                    generatedBuffer.setSize(2, numSamples, false, false, true);

                    unisonPairCount = (int)synthParameters.unisonCount->load();
                    unisonGain = synthParameters.unisonGain->load() / 100.f;

                    /*render buffer*/
                    for (size_t channel = 0; channel < 2; channel++)
                    {
                        auto* bufferPointer = generatedBuffer.getWritePointer(channel, 0);

                        for (size_t sample = 0; sample < numSamples; sample++)
                        {
                            //Generating the fundamental data for the sample
                            bufferPointer[sample] += getFundamentalSample(channel);

                            //Generating unison data for the sample
                            if( unisonGain > 0.f )
                            {
                                for (size_t unison = 0; unison < unisonPairCount; unison++)
                                {
                                    bufferPointer[sample] += getUnisonSample(channel, unison);
                                }
                            }
                        }
                    }

                    //Applying the envelope to the buffer
                    amplitudeADSR.applyEnvelopeToBuffer(generatedBuffer, 0, numSamples);
                    
                    for (size_t channel = 0; channel < 2; channel++)
                    {
                        outputBuffer.addFrom(channel, startSample, generatedBuffer, channel, 0, numSamples);

                        if (!amplitudeADSR.isActive())
                        {
                            clearCurrentNote();
                        }
                    }
                }
                updateFrequencies();
                updateAngles();
            }
        }

    private:
        juce::AudioBuffer<float> generatedBuffer;
        AdditiveSynthParameters& synthParameters;
        
        juce::Random rng;

        const juce::OwnedArray<juce::dsp::LookupTableTransform<float>>& mipMap;

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
        const float getFundamentalSample(const int channel)
        {
            if (voiceData.currentAngle[channel] > juce::MathConstants<float>::twoPi)
            {
                voiceData.currentAngle[channel] -= juce::MathConstants<float>::twoPi;
            }

            //Generating the fundamental data for the sample
            float sample = velocityGain * (*mipMap[mipMapIndex])[voiceData.currentAngle[channel]];

            voiceData.currentAngle[channel] += voiceData.angleDelta;

            return sample;
        }

        /// @brief Generates a sample for the unison of the voice
        /// @param channel The channel where the sample is needed (for random phase per channel)
        /// @param unisonNumber The unison pair to generate with
        /// @return The generated sample
        const float getUnisonSample(const int channel, const int unisonNumber)
        {
            if (voiceData.unisonData[unisonNumber].upperCurrentAngle[channel] > juce::MathConstants<float>::twoPi)
            {
                voiceData.unisonData[unisonNumber].upperCurrentAngle[channel] -= juce::MathConstants<float>::twoPi;
            }
            if (voiceData.unisonData[unisonNumber].lowerCurrentAngle[channel] > juce::MathConstants<float>::twoPi)
            {
                voiceData.unisonData[unisonNumber].lowerCurrentAngle[channel] -= juce::MathConstants<float>::twoPi;
            }

            float sample = velocityGain * unisonGain * (*mipMap[mipMapIndex])[voiceData.unisonData[unisonNumber].upperCurrentAngle[channel]];
            sample += velocityGain * unisonGain * (*mipMap[mipMapIndex])[voiceData.unisonData[unisonNumber].lowerCurrentAngle[channel]];

            voiceData.unisonData[unisonNumber].upperCurrentAngle[channel] += voiceData.unisonData[unisonNumber].upperAngleDelta;
            voiceData.unisonData[unisonNumber].lowerCurrentAngle[channel] += voiceData.unisonData[unisonNumber].lowerAngleDelta;

            return sample;
        }

        /// @brief Used to randomise the starting phases of all generated waveforms
        void updatePhases()
        {
            unisonPairCount = synthParameters.unisonCount->load();
            for (size_t channel = 0; channel < 2; channel++)
            {
                voiceData.currentAngle[channel] = getRandomPhase() + ( ( synthParameters.globalPhase->load() / 100 ) * juce::MathConstants<float>::twoPi );
                for (size_t unison = 0; unison < unisonPairCount; unison++)
                {
                    voiceData.unisonData[unison].upperCurrentAngle[channel] = getRandomPhase() + ( ( synthParameters.globalPhase->load() / 100 ) * juce::MathConstants<float>::twoPi );
                    voiceData.unisonData[unison].lowerCurrentAngle[channel] = getRandomPhase() + ( ( synthParameters.globalPhase->load() / 100 ) * juce::MathConstants<float>::twoPi );
                }
            }       
        }

        /// @brief Used to generate a random phase offset based on the configured starting position and range
        /// @return Returns the generated offset
        const float getRandomPhase()
        {
            return ((rng.nextFloat() * synthParameters.randomPhaseRange->load() / 100) * juce::MathConstants<float>::twoPi);
        }

        /// @brief Called to update frequencies with current parameters
        void updateFrequencies()
        {
            //formula for equal temperament from midi note# with A4 at 440Hz
            voiceData.frequency = 440.f * pow(2, ((float)currentNote - 69.f) / 12);

            //Applying octave, semitone and fine tuning and pitchwheel offsets
            float unifiedGlobalTuningOffset = pow(2, synthParameters.oscillatorOctaves->load() + (synthParameters.oscillatorSemitones->load() / 12) + (synthParameters.oscillatorFine->load() / 1200) + (synthParameters.pitchWheelRange->load() * pitchWheelOffset / 12));
            voiceData.frequency *= unifiedGlobalTuningOffset;

            /*Calculating evenly spaced unison frequency offsets and applying the global tuning offset*/
            unisonPairCount = synthParameters.unisonCount->load();
            float unisonTuningRange = pow(2, synthParameters.unisonDetune->load() / 1200);
            float unisonTuningStep = (unisonTuningRange - 1) / unisonPairCount;

            for (size_t unison = 0; unison < unisonPairCount; unison++)
            {
                voiceData.unisonData[unison].upperFrequencyOffset = 1.f + (unisonTuningStep * (unison + 1));
                voiceData.unisonData[unison].lowerFrequencyOffset = 1.f / ( 1.f + (unisonTuningStep * (unison + 1)));
            }

            if (unisonPairCount > 0)
            {
                highestCurrentFrequency = voiceData.frequency * unisonTuningRange;
            }
            else
            {
                highestCurrentFrequency = voiceData.frequency;
            }

            findMipMapToUse();
        }

        /// @brief Used to generate new angle deltas for the given frequencies the voice is expected to generate
        void updateAngles()
        {
            auto sampleRate = getSampleRate();
            float cyclesPerSample = voiceData.frequency / sampleRate;
            voiceData.angleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;

            unisonPairCount = synthParameters.unisonCount->load();
            for (size_t unison = 0; unison < unisonPairCount; unison++)
            {
                cyclesPerSample = (voiceData.frequency * voiceData.unisonData[unison].upperFrequencyOffset) / sampleRate;
                voiceData.unisonData[unison].upperAngleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;

                cyclesPerSample = (voiceData.frequency * voiceData.unisonData[unison].lowerFrequencyOffset) / sampleRate;
                voiceData.unisonData[unison].lowerAngleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;
            }
        }

        /// @brief Checks the highest possible overtone the current highest generated frequency (including the up-tuned unison voices) that can safely be generated without aliasing at the current sample-rate and selects the right lookup table with the correct number of overtones. Playback is skipped entirely if such a look-up table doesn't exist
        void findMipMapToUse()
        {
            float highestGeneratedOvertone = getSampleRate();
            mipMapIndex = -1;
            while (highestGeneratedOvertone >= (getSampleRate() / 2) && mipMapIndex < LOOKUP_SIZE)
            {
                mipMapIndex++;
                highestGeneratedOvertone = highestCurrentFrequency * std::ceilf((HARMONIC_N / pow(2.f, (float)mipMapIndex)));
            }

            if(mipMapIndex >= LOOKUP_SIZE)
            {   //The lowest usable frequency exceeds or equals the Nyquist frequency. None of the generated signal would be valid data at this point
                bypassPlaying = true;
            }
            else
            {
                bypassPlaying = false;
            }
        }

        /// @brief Used to stop playback and reset values when a note off message arrives
        void resetProperties()
        {
            generatedBuffer.clear();

            velocityGain = 0;
            currentNote = 0;
            bypassPlaying = false;

            unisonPairCount = 0;
            unisonGain = 0.f;

            pitchWheelOffset = 0.f;

            highestCurrentFrequency = 0.f;

            voiceData.reset();
        }

        void updateADSRParams()
        {
            juce::ADSR::Parameters params;

            amplitudeADSR.setSampleRate(getSampleRate());

            params.attack = synthParameters.amplitudeADSRAttack->load() / 1000;
            params.decay = synthParameters.amplitudeADSRDecay->load() / 1000;
            params.sustain = synthParameters.amplitudeADSRSustain->load() / 100;
            params.release = synthParameters.amplitudeADSRRelease->load() / 1000;

            amplitudeADSR.setParameters(params);
        }
    };
}
