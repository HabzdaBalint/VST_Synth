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
    AdditiveVoice(SynthParameters& params, juce::dsp::LookupTableTransform<float>& lut) : synthParameters(&params), lut(&lut) {}

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
        //formula for equal temperament from midi note# with A4 at 440Hz
        fundamentalFrequency = 440.f * pow(2, ((float)midiNoteNumber - 69.f) / 12);

        velocityGain = velocity;

        randomisePhases();
        unisonPairCount = synthParameters->unisonPairCount;
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
            /*Generating the fundamental data for the sample*/

            leftBufferPointer[sample] += velocityGain * lut->operator()(wrapBackInRange(0, juce::MathConstants<float>::twoPi,fundamentalCurrentAngle + fundamentalPhaseOffset[0] * juce::MathConstants<float>::pi));
            rightBufferPointer[sample] += velocityGain * lut->operator()(wrapBackInRange(0, juce::MathConstants<float>::twoPi,fundamentalCurrentAngle + fundamentalPhaseOffset[1] * juce::MathConstants<float>::pi));

            fundamentalCurrentAngle += fundamentalAngleDelta;
            if (fundamentalCurrentAngle > juce::MathConstants<float>::twoPi)
            {
                fundamentalCurrentAngle -= juce::MathConstants<float>::twoPi;
            }

            /*Generating unison data for the sample*/
            for (size_t i = 0; i < 2 * unisonPairCount; i++)
            {
                leftBufferPointer[sample] += velocityGain * synthParameters->unisonGain * lut->operator()(wrapBackInRange(0, juce::MathConstants<float>::twoPi, unisonCurrentAngles[i] + unisonPhaseOffsets[0][i] * juce::MathConstants<float>::pi));
                rightBufferPointer[sample] += velocityGain * synthParameters->unisonGain * lut->operator()(wrapBackInRange(0, juce::MathConstants<float>::twoPi, unisonCurrentAngles[i] + unisonPhaseOffsets[1][i] * juce::MathConstants<float>::pi));

                unisonCurrentAngles[i] += unisonAngleDeltas[i];

                if (unisonCurrentAngles[i] > juce::MathConstants<float>::twoPi)
                {
                    unisonCurrentAngles[i] -= juce::MathConstants<float>::twoPi;
                }
            }
        }

        /*adsr*/
        unisonPairCount = synthParameters->unisonPairCount;
        updateFrequencies();
        updateAngles();
    }

    void randomisePhases()
    {
        /*generate phase offsets*/
    }

    void updateAngles()
    {
        auto sampleRate = getSampleRate();
        float cyclesPerSample = fundamentalFrequency / sampleRate;
        fundamentalAngleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;

        for (size_t i = 0; i < 2 * unisonPairCount; i += 2)
        {
            cyclesPerSample = fundamentalFrequency * unisonFrequencyOffsets[i] / sampleRate;
            unisonAngleDeltas[i] = cyclesPerSample * juce::MathConstants<float>::twoPi;
            cyclesPerSample = fundamentalFrequency / unisonFrequencyOffsets[i] / sampleRate;
            unisonAngleDeltas[i + 1] = cyclesPerSample * juce::MathConstants<float>::twoPi;
        }
    }

    void updateFrequencies()
    {
        //Applying octave, semitone and fine tuning in one power function 
        float unifiedGlobalTuningOffset = pow(2, synthParameters->octaveTuning + (float)synthParameters->semitoneTuning / 12 + (float)synthParameters->fineTuningCents / 1200);
        fundamentalFrequency *= unifiedGlobalTuningOffset;

        /*calculating evenly spaced unison frequency offsets and applying the global tuning offset*/
        for (size_t i = 0; i < unisonPairCount; i++)
        {
            float symmetricUnisonTuningOffset = (float)synthParameters->unisonDetune / (i + 1);
            unisonFrequencyOffsets[i] = symmetricUnisonTuningOffset * unifiedGlobalTuningOffset;
        }
    }

    /*fast, unchecked helper function*/
    float wrapBackInRange(float lowerBound, float upperBound, float value)
    {
        return fmod(value, upperBound - lowerBound);
    }

    /*only call if rendering is guaranteed to be stopped*/
    void resetProperties()
    {
        velocityGain = 0;
        fundamentalCurrentAngle = 0;
        fundamentalAngleDelta = 0;
        fundamentalFrequency = 0;
        fundamentalPhaseOffset[0] = 0;
        fundamentalPhaseOffset[1] = 0;

        unisonPairCount = 0;
        for (size_t i = 0; i < 10; i++)
        {
            unisonAngleDeltas[i] = 0;
            unisonCurrentAngles[i] = 0;
            unisonPhaseOffsets[0][i] = 0;
            unisonPhaseOffsets[1][i] = 0;
        }
        for (size_t i = 0; i < 5; i++)
        {
            unisonFrequencyOffsets[i] = 0;
        }
    }

private:
    SynthParameters* synthParameters;
    juce::dsp::LookupTableTransform<float>* lut;

    float velocityGain = 0;

    float fundamentalCurrentAngle = 0;
    float fundamentalAngleDelta = 0;
    float fundamentalFrequency = 0;
    float fundamentalPhaseOffset[2] = { 0,0 };

    int unisonPairCount = 0;
    float unisonCurrentAngles[10] = { 0,0,0,0,0,0,0,0,0,0 };
    float unisonAngleDeltas[10] = { 0,0,0,0,0,0,0,0,0,0 };
    float unisonFrequencyOffsets[5] = { 0,0,0,0,0 };
    float unisonPhaseOffsets[2][10] = { { 0,0,0,0,0,0,0,0,0,0 }, { 0,0,0,0,0,0,0,0,0,0 } };
};