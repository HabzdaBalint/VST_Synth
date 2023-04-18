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
    class AdditiveVoice : public juce::SynthesiserVoice
    {
    public:
        AdditiveVoice(AdditiveSynthParameters& synthParams, juce::OwnedArray<juce::dsp::LookupTableTransform<float>>& mipMap) :
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
            amplitudeADSR.noteOff();
            
            if (!allowTailOff || !amplitudeADSR.isActive())
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
            if( isVoiceActive() && !bypassPlaying && velocityGain != 0.f )
            {
                generatedBuffer.clear();
                generatedBuffer.setSize(2, numSamples, false, false, true);

                /*render buffer, apply gain*/
                for (size_t channel = 0; channel < 2; channel++)
                {
                    auto* bufferPointer = generatedBuffer.getWritePointer(channel, 0);

                    for (size_t sample = 0; sample < numSamples; sample++)
                    {
                        if (fundamentalCurrentAngle[channel] > juce::MathConstants<float>::twoPi)
                        {
                            fundamentalCurrentAngle[channel] -= juce::MathConstants<float>::twoPi;
                        }

                        //Generating the fundamental data for the sample
                        bufferPointer[sample] += velocityGain * mipMap[mipMapIndex]->operator()(fundamentalCurrentAngle[channel]);

                        fundamentalCurrentAngle[channel] += fundamentalAngleDelta;

                        //Generating unison data for the sample
                        if(synthParameters.unisonGain->load() > 0.f)
                        {
                            for (size_t unison = 0; unison < synthParameters.unisonCount->load(); unison++)
                            {
                                if (unisonCurrentAngles[channel][unison] > juce::MathConstants<float>::twoPi)
                                {
                                    unisonCurrentAngles[channel][unison] -= juce::MathConstants<float>::twoPi;
                                }

                                bufferPointer[sample] += velocityGain * ( synthParameters.unisonGain->load() / 100 ) * mipMap[mipMapIndex]->operator()(unisonCurrentAngles[channel][unison]);

                                unisonCurrentAngles[channel][unison] += unisonAngleDeltas[unison];
                            }
                        }

                        //Applying the envelope to the sample
                        bufferPointer[sample] *= amplitudeADSR.getNextSample();
                    }
                }
                
                for (size_t channel = 0; channel < 2; channel++)
                {
                    outputBuffer.addFrom (channel, startSample, generatedBuffer, channel, 0, numSamples);

                    if (!amplitudeADSR.isActive())
                    {
                        clearCurrentNote();
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

        juce::OwnedArray<juce::dsp::LookupTableTransform<float>>& mipMap;
        int mipMapIndex = 0;

        float velocityGain = 0;
        int currentNote = 0;
        bool bypassPlaying = false;

        float pitchWheelOffset = 0;

        float fundamentalCurrentAngle[2] = { 0,0 };
        float fundamentalAngleDelta = 0;
        float fundamentalFrequency = 0;

        float unisonCurrentAngles[2][10] = { { 0,0,0,0,0,0,0,0,0,0 }, { 0,0,0,0,0,0,0,0,0,0 } };
        float unisonAngleDeltas[10] = { 0,0,0,0,0,0,0,0,0,0 };
        float unisonFrequencyOffsets[5] = { 0,0,0,0,0 };

        float highestCurrentFrequency = 0;

        juce::ADSR amplitudeADSR;

        /// @brief Used to randomise the starting phases of all generated waveforms
        void updatePhases()
        {
            for (size_t channel = 0; channel < 2; channel++)
            {
                fundamentalCurrentAngle[channel] = getRandomPhase();
                for (size_t i = 0; i < 2 * synthParameters.unisonCount->load(); i++)
                {
                    unisonCurrentAngles[channel][i] = getRandomPhase();
                }
            }       
        }

        /// @brief Used to generate a random phase offset based on the configured starting position and range
        /// @return Returns the generated offset
        float getRandomPhase()
        {
            return (((rng.nextFloat() * synthParameters.randomPhaseRange->load() / 100) + ( synthParameters.globalPhase->load() / 100 ) ) * juce::MathConstants<float>::twoPi);
        }

        /// @brief Called to update frequencies with current parameters
        void updateFrequencies()
        {
            //formula for equal temperament from midi note# with A4 at 440Hz
            fundamentalFrequency = 440.f * pow(2, ((float)currentNote - 69.f) / 12);
            //Applying octave, semitone and fine tuning and pitchwheel offsets
            float unifiedGlobalTuningOffset = pow(2, synthParameters.oscillatorOctaves->load() + (synthParameters.oscillatorSemitones->load() / 12) + (synthParameters.oscillatorFine->load() / 1200) + (synthParameters.pitchWheelRange->load() * pitchWheelOffset / 12));
            fundamentalFrequency *= unifiedGlobalTuningOffset;

            /*Calculating evenly spaced unison frequency offsets and applying the global tuning offset*/
            float unisonTuningRange = pow(2, synthParameters.unisonDetune->load() / 1200);
            float unisonTuningStep = (unisonTuningRange - 1) / synthParameters.unisonCount->load();
            for (size_t i = 0; i < synthParameters.unisonCount->load(); i++)
            {
                unisonFrequencyOffsets[i] = 1 + (unisonTuningStep * (i + 1));
            }

            if (synthParameters.unisonCount->load() > 0)
            {
                highestCurrentFrequency = fundamentalFrequency * unisonTuningRange;
            }
            else
            {
                highestCurrentFrequency = fundamentalFrequency;
            }

            findMipMapToUse();
        }

        /// @brief Used to generate new angle deltas for the given frequencies the voice is expected to generate
        void updateAngles()
        {
            auto sampleRate = getSampleRate();
            float cyclesPerSample = fundamentalFrequency / sampleRate;
            fundamentalAngleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;

            for (size_t i = 0; i < (2 * synthParameters.unisonCount->load()); i += 2)
            {
                cyclesPerSample = (fundamentalFrequency * unisonFrequencyOffsets[i]) / sampleRate;
                unisonAngleDeltas[i] = cyclesPerSample * juce::MathConstants<float>::twoPi;
                cyclesPerSample = (fundamentalFrequency / unisonFrequencyOffsets[i]) / sampleRate;
                unisonAngleDeltas[i + 1] = cyclesPerSample * juce::MathConstants<float>::twoPi;
            }
        }

        /// @brief Checks the highest possible overtone the current highest generated frequency (including the up-tuned unison waveforms) that can safely be generated without aliasing at the current sample-rate and selects the right lookup table with the correct number of overtones. Playback is skipped entirely if such a look-up table doesn't exist
        void findMipMapToUse()
        {
            float highestGeneratedOvertone = getSampleRate();
            mipMapIndex = -1;
            while (highestGeneratedOvertone >= (getSampleRate() / 2) && mipMapIndex < LOOKUP_SIZE)
            {
                mipMapIndex++;
                highestGeneratedOvertone = highestCurrentFrequency * (HARMONIC_N / pow(2, mipMapIndex));
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

            pitchWheelOffset = 0;

            fundamentalCurrentAngle[0] = 0;
            fundamentalCurrentAngle[1] = 0;
            fundamentalAngleDelta = 0;
            fundamentalFrequency = 0;

            for (size_t i = 0; i < 10; i++)
            {
                unisonAngleDeltas[i] = 0;
                unisonCurrentAngles[0][i] = 0;
                unisonCurrentAngles[1][i] = 0;
            }
            for (size_t i = 0; i < 5; i++)
            {
                unisonFrequencyOffsets[i] = 0;
            }
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
