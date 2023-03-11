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
    AdditiveVoice(SynthParameters& params, juce::Array<juce::dsp::LookupTableTransform<float>*>& mipMap, int mipMapSize) : synthParameters(&params), mipMap(mipMap), mipMapSize(mipMapSize) {}

    ~AdditiveVoice() {}

    bool canPlaySound(juce::SynthesiserSound* sound) override { return sound != nullptr; }

    void controllerMoved(int controllerNumber, int newControllerValue) override { }

    //Pitch Wheel is allowed to modify the tone by 2 semitones up and 2 semitones down
    void pitchWheelMoved(int newPitchWheelValue) override
    {
        //todo
    }

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override
    {
        currentNote = midiNoteNumber;
        velocityGain = velocity;

        updatePhases();
        updateFrequencies();
        updateAngles();

        /*adsr*/
    }

    void stopNote(float velocity, bool allowTailOff) override
    {
        /*adsr*/
        //if (!allowTailOff) etc
        clearCurrentNote();
        resetProperties();
    }

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        /*render loop, apply gain*/
        auto* leftBufferPointer = outputBuffer.getWritePointer(0, startSample);
        auto* rightBufferPointer = outputBuffer.getWritePointer(1, startSample);

        for (size_t sample = 0; sample < numSamples; sample++)
        {
            if (fundamentalCurrentAngle[0] > juce::MathConstants<float>::twoPi)
            {
                fundamentalCurrentAngle[0] -= juce::MathConstants<float>::twoPi;
            }
            if (fundamentalCurrentAngle[1] > juce::MathConstants<float>::twoPi)
            {
                fundamentalCurrentAngle[1] -= juce::MathConstants<float>::twoPi;
            }

            /*Generating the fundamental data for the sample*/
            leftBufferPointer[sample] += velocityGain * mipMap[mipMapIndex]->operator()(fundamentalCurrentAngle[0]);
            rightBufferPointer[sample] += velocityGain * mipMap[mipMapIndex]->operator()(fundamentalCurrentAngle[1]);

            fundamentalCurrentAngle[0] += fundamentalAngleDelta;
            fundamentalCurrentAngle[1] += fundamentalAngleDelta;

            /*Generating unison data for the sample*/
            for (size_t i = 0; i < synthParameters->unisonPairCount; i++)
            {
                if (unisonCurrentAngles[0][i] > juce::MathConstants<float>::twoPi)
                {
                    unisonCurrentAngles[0][i] -= juce::MathConstants<float>::twoPi;
                }
                if (unisonCurrentAngles[1][i] > juce::MathConstants<float>::twoPi)
                {
                    unisonCurrentAngles[1][i] -= juce::MathConstants<float>::twoPi;
                }

                leftBufferPointer[sample] += velocityGain * synthParameters->unisonGain * mipMap[mipMapIndex]->operator()(unisonCurrentAngles[0][i]);
                rightBufferPointer[sample] += velocityGain * synthParameters->unisonGain * mipMap[mipMapIndex]->operator()(unisonCurrentAngles[1][i]);

                unisonCurrentAngles[0][i] += unisonAngleDeltas[i];
                unisonCurrentAngles[1][i] += unisonAngleDeltas[i];
            }
        }

        /*adsr*/
        updateFrequencies();
        updateAngles();
    }

    /*todo: random phase*/
    void updatePhases()
    {
        fundamentalCurrentAngle[0] = getRandomPhase();
        fundamentalCurrentAngle[1] = getRandomPhase();

        for (size_t i = 0; i < 2 * synthParameters->unisonPairCount; i++)
        {
            unisonCurrentAngles[0][i] = getRandomPhase();
            unisonCurrentAngles[1][i] = getRandomPhase();
        }
    }

    float getRandomPhase()
    {
        return (rng.nextFloat() * synthParameters->randomPhaseRange * juce::MathConstants<float>::twoPi) + (synthParameters->globalPhseStart * juce::MathConstants<float>::pi);
    }

    /*Updating frequencies in case of getting a new note or another tuning parameter change*/
    void updateFrequencies()
    {
        //formula for equal temperament from midi note# with A4 at 440Hz
        fundamentalFrequency = 440.f * pow(2, ((float)currentNote - 69.f) / 12);
        //Applying octave, semitone and fine tuning in one power function 
        float unifiedGlobalTuningOffset = pow(2, synthParameters->octaveTuning + ((float)synthParameters->semitoneTuning / 12) + ((float)synthParameters->fineTuningCents / 1200));
        fundamentalFrequency *= unifiedGlobalTuningOffset;

        /*Calculating evenly spaced unison frequency offsets and applying the global tuning offset*/
        float unisonTuningRange = pow(2, (float)synthParameters->unisonDetune / 1200);
        float unisonTuningStep = (unisonTuningRange - 1) / synthParameters->unisonPairCount;
        for (size_t i = 0; i < synthParameters->unisonPairCount; i++)
        {
            unisonFrequencyOffsets[i] = 1 + (unisonTuningStep * (i + 1));
        }

        if (synthParameters->unisonPairCount > 0)
        {
            highestCurrentFrequency = fundamentalFrequency * unisonTuningRange;
        }
        else
        {
            highestCurrentFrequency = fundamentalFrequency;
        }

        findMipMapToUse();
    }

    /*Updating angles to match new frequencies*/
    void updateAngles()
    {
        auto sampleRate = getSampleRate();
        float cyclesPerSample = fundamentalFrequency / sampleRate;
        fundamentalAngleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;

        for (size_t i = 0; i < (2 * synthParameters->unisonPairCount); i += 2)
        {
            cyclesPerSample = (fundamentalFrequency * unisonFrequencyOffsets[i]) / sampleRate;
            unisonAngleDeltas[i] = cyclesPerSample * juce::MathConstants<float>::twoPi;
            cyclesPerSample = (fundamentalFrequency / unisonFrequencyOffsets[i]) / sampleRate;
            unisonAngleDeltas[i + 1] = cyclesPerSample * juce::MathConstants<float>::twoPi;
        }
    }

    /*Checks the highest possible overtone the current highest generated frequency can safely generate without aliasing*/
    void findMipMapToUse()
    {
        float highestGeneratedOvertone = getSampleRate();
        mipMapIndex = -1;
        while (highestGeneratedOvertone >= (getSampleRate() / 2) && mipMapIndex < mipMapSize)
        {
            mipMapIndex++;
            highestGeneratedOvertone = highestCurrentFrequency * (HARMONIC_N / pow(2, mipMapIndex));
        }
    }

    /*only call if rendering is guaranteed to be stopped*/
    void resetProperties()
    {
        velocityGain = 0;
        currentNote = 0;
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

private:
    SynthParameters* synthParameters;

    juce::Array<juce::dsp::LookupTableTransform<float>*>& mipMap;
    int mipMapSize = 0;

    float velocityGain = 0;
    float currentNote = 0;

    juce::Random rng;

    float fundamentalCurrentAngle[2] = { 0,0 };
    float fundamentalAngleDelta = 0;
    float fundamentalFrequency = 0;

    float unisonCurrentAngles[2][10] = { { 0,0,0,0,0,0,0,0,0,0 }, { 0,0,0,0,0,0,0,0,0,0 } };
    float unisonAngleDeltas[10] = { 0,0,0,0,0,0,0,0,0,0 };
    float unisonFrequencyOffsets[5] = { 0,0,0,0,0 };

    float highestCurrentFrequency = 0;
    int mipMapIndex = 0;
};