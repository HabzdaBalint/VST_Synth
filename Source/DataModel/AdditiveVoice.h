/*
  ==============================================================================

    AdditiveVoice.h
    Created: 11 Mar 2023 2:17:04am
    Author:  Habama10

  ==============================================================================
*/

#pragma once

#include "SynthParameters.h"

class AdditiveVoice : public juce::SynthesiserVoice
{
public:
    AdditiveVoice(SynthParametersAtomic& synthParamsAtomic, juce::Array<juce::dsp::LookupTableTransform<float>*>& mipMap) :
        synthParametersAtomic(&synthParamsAtomic),
        mipMap(mipMap) {}

    ~AdditiveVoice() {}

    bool canPlaySound(juce::SynthesiserSound* sound) override { return sound != nullptr; }

    void controllerMoved(int controllerNumber, int newControllerValue) override { }

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

        amplitudeADSR.reset();
        updateADSRParams();
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
    {
        /*render buffer, apply gain*/
        generatedBuffer.clear();
        generatedBuffer.setSize(2, numSamples, false, false, true);

        for (size_t channel = 0; channel < 2; channel++)
        {
            auto* bufferPointer = generatedBuffer.getWritePointer(channel, 0);

            for (size_t sample = 0; sample < numSamples; sample++)
            {
                if (fundamentalCurrentAngle[channel] > juce::MathConstants<float>::twoPi)
                {
                    fundamentalCurrentAngle[channel] -= juce::MathConstants<float>::twoPi;
                }

                /*Generating the fundamental data for the sample*/
                bufferPointer[sample] += velocityGain * mipMap[mipMapIndex]->operator()(fundamentalCurrentAngle[channel]);

                fundamentalCurrentAngle[channel] += fundamentalAngleDelta;

                /*Generating unison data for the sample*/
                for (size_t unison = 0; unison < synthParametersAtomic->unisonPairCount; unison++)
                {
                    if (unisonCurrentAngles[channel][unison] > juce::MathConstants<float>::twoPi)
                    {
                        unisonCurrentAngles[channel][unison] -= juce::MathConstants<float>::twoPi;
                    }

                    bufferPointer[sample] += velocityGain * synthParametersAtomic->unisonGain * mipMap[mipMapIndex]->operator()(unisonCurrentAngles[channel][unison]);

                    unisonCurrentAngles[channel][unison] += unisonAngleDeltas[unison];
                }
                bufferPointer[sample] *= amplitudeADSR.getNextSample();
            }
        }
        
        for (size_t channel = 0; channel < 2; channel++)
        {
            outputBuffer.addFrom (channel, startSample, generatedBuffer, channel, 0, numSamples);

            if (! amplitudeADSR.isActive())
            {
                clearCurrentNote();
            }
        }

        updateADSRParams();
        updateFrequencies();
        updateAngles();
    }

    /// @brief Used to randomise the starting phases of all generated waveforms
    void updatePhases()
    {
        for (size_t channel = 0; channel < 2; channel++)
        {
            fundamentalCurrentAngle[channel] = getRandomPhase();
            for (size_t i = 0; i < 2 * synthParametersAtomic->unisonPairCount; i++)
            {
                unisonCurrentAngles[channel][i] = getRandomPhase();
            }
        }       
    }

    /// @brief Used to generate a random phase offset based on the configured starting position and range
    /// @return Returns the generated offset
    float getRandomPhase()
    {
        return (((rng.nextFloat() * synthParametersAtomic->randomPhaseRange) + synthParametersAtomic->globalPhseStart) * juce::MathConstants<float>::twoPi);
    }

    /*Updating frequencies in case of receiving a new note or a tuning parameter change or pitch wheel event*/
    void updateFrequencies()
    {
        //formula for equal temperament from midi note# with A4 at 440Hz
        fundamentalFrequency = 440.f * pow(2, ((float)currentNote - 69.f) / 12);
        //Applying octave, semitone and fine tuning and pitchwheel offsets
        float unifiedGlobalTuningOffset = pow(2, synthParametersAtomic->octaveTuning + (synthParametersAtomic->semitoneTuning / 12) + (synthParametersAtomic->fineTuningCents / 1200) + (synthParametersAtomic->pitchWheelRange * pitchWheelOffset / 12));
        fundamentalFrequency *= unifiedGlobalTuningOffset;

        /*Calculating evenly spaced unison frequency offsets and applying the global tuning offset*/
        float unisonTuningRange = pow(2, synthParametersAtomic->unisonDetune / 1200);
        float unisonTuningStep = (unisonTuningRange - 1) / synthParametersAtomic->unisonPairCount;
        for (size_t i = 0; i < synthParametersAtomic->unisonPairCount; i++)
        {
            unisonFrequencyOffsets[i] = 1 + (unisonTuningStep * (i + 1));
        }

        if (synthParametersAtomic->unisonPairCount > 0)
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

        for (size_t i = 0; i < (2 * synthParametersAtomic->unisonPairCount); i += 2)
        {
            cyclesPerSample = (fundamentalFrequency * unisonFrequencyOffsets[i]) / sampleRate;
            unisonAngleDeltas[i] = cyclesPerSample * juce::MathConstants<float>::twoPi;
            cyclesPerSample = (fundamentalFrequency / unisonFrequencyOffsets[i]) / sampleRate;
            unisonAngleDeltas[i + 1] = cyclesPerSample * juce::MathConstants<float>::twoPi;
        }
    }

    /// @brief Checks the highest possible overtone the current highest generated frequency can safely generate without aliasing and selects the right lookup table with the correct number of overtones
    void findMipMapToUse()
    {
        float highestGeneratedOvertone = getSampleRate();
        mipMapIndex = -1;
        while (highestGeneratedOvertone >= (getSampleRate() / 2) && mipMapIndex < LOOKUP_SIZE)
        {
            mipMapIndex++;
            highestGeneratedOvertone = highestCurrentFrequency * (HARMONIC_N / pow(2, mipMapIndex));
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

        params.attack = synthParametersAtomic->attack;
        params.decay = synthParametersAtomic->decay;
        params.sustain = synthParametersAtomic->sustain;
        params.release = synthParametersAtomic->release;

        amplitudeADSR.setParameters(params);
    }

    juce::ADSR amplitudeADSR;
private:
    juce::AudioBuffer<float> generatedBuffer;
    SynthParametersAtomic* synthParametersAtomic;
    
    juce::Random rng;

    juce::Array<juce::dsp::LookupTableTransform<float>*>& mipMap;

    float velocityGain = 0;
    float currentNote = 0;

    float pitchWheelOffset = 0;

    float fundamentalCurrentAngle[2] = { 0,0 };
    float fundamentalAngleDelta = 0;
    float fundamentalFrequency = 0;

    float unisonCurrentAngles[2][10] = { { 0,0,0,0,0,0,0,0,0,0 }, { 0,0,0,0,0,0,0,0,0,0 } };
    float unisonAngleDeltas[10] = { 0,0,0,0,0,0,0,0,0,0 };
    float unisonFrequencyOffsets[5] = { 0,0,0,0,0 };

    float highestCurrentFrequency = 0;
    int mipMapIndex = 0;
};