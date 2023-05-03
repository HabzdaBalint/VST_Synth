/*
==============================================================================

    AdditiveVoice.cpp
    Created: 26 Apr 2023 3:23:12pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "AdditiveSynthParameters.h"
#include "AdditiveVoice.h"

namespace Processor::Synthesizer
{
    AdditiveVoice::AdditiveVoice(
        AdditiveSynthParameters& synthParams,
        const juce::OwnedArray<Utils::TripleBuffer<juce::dsp::LookupTableTransform<float>>>& mipMap) :
            synthParameters(synthParams),
            mipMap(mipMap)
    {}

    bool AdditiveVoice::isVoiceActive() const
    {
        return ( getCurrentlyPlayingNote() >= 0 || amplitudeADSR.isActive() );
    }

    void AdditiveVoice::pitchWheelMoved(int newPitchWheelValue)
    {
        pitchWheelOffset = ((float)newPitchWheelValue-8192)/8192;

        updateFrequencies();
        updateAngles();
    }

    void AdditiveVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition)
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

    void AdditiveVoice::stopNote(float velocity, bool allowTailOff)
    {
        if( !isSustainPedalDown() && !isSostenutoPedalDown() )
            amplitudeADSR.noteOff();
        
        if( !allowTailOff || !amplitudeADSR.isActive() )
        {
            clearCurrentNote();
            resetProperties();
        }
    }

    void AdditiveVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
    {   //No point in updating variables and calculating samples if velocity is 0 or if the voice is not in use
        if( isVoiceActive() && velocityGain > 0.f )
        {
            if( !bypassPlaying )
            {
                generatedBuffer.clear();
                generatedBuffer.setSize(2, numSamples, false, false, true);

                unisonPairCount = (int)synthParameters.unisonCount->load();
                unisonGain = synthParameters.unisonGain->load() / 100.f;

                mipMap[mipMapIndex]->acquire();
                auto& localMipMap = mipMap[mipMapIndex]->read();

                /*render buffer*/
                for (int channel = 0; channel < 2; channel++)
                {
                    auto* bufferPointer = generatedBuffer.getWritePointer(channel, 0);

                    for (int sample = 0; sample < numSamples; sample++)
                    {
                        //Generating the fundamental data for the sample
                        bufferPointer[sample] += getFundamentalSample(channel, localMipMap);

                        //Generating unison data for the sample
                        if( unisonGain > 0.f )
                        {
                            for (int unison = 0; unison < unisonPairCount; unison++)
                            {
                                bufferPointer[sample] += getUnisonSample(channel, unison, localMipMap);
                            }
                        }
                    }
                }

                //Applying the envelope to the buffer
                amplitudeADSR.applyEnvelopeToBuffer(generatedBuffer, 0, numSamples);
                
                for (int channel = 0; channel < 2; channel++)
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

    const float AdditiveVoice::getFundamentalSample(const int channel, const juce::dsp::LookupTableTransform<float>& localMipMap)
    {
        if (voiceData.currentAngle[channel] > juce::MathConstants<float>::twoPi)
        {
            voiceData.currentAngle[channel] -= juce::MathConstants<float>::twoPi;
        }

        //Generating the fundamental data for the sample
        float sample = velocityGain * localMipMap[voiceData.currentAngle[channel]];

        voiceData.currentAngle[channel] += voiceData.angleDelta;

        return sample;
    }

    const float AdditiveVoice::getUnisonSample(const int channel, const int unisonNumber, const juce::dsp::LookupTableTransform<float>& localMipMap)
    {
        if (voiceData.unisonData[unisonNumber].upperCurrentAngle[channel] > juce::MathConstants<float>::twoPi)
        {
            voiceData.unisonData[unisonNumber].upperCurrentAngle[channel] -= juce::MathConstants<float>::twoPi;
        }
        if (voiceData.unisonData[unisonNumber].lowerCurrentAngle[channel] > juce::MathConstants<float>::twoPi)
        {
            voiceData.unisonData[unisonNumber].lowerCurrentAngle[channel] -= juce::MathConstants<float>::twoPi;
        }

        float sample = velocityGain * unisonGain * localMipMap[voiceData.unisonData[unisonNumber].upperCurrentAngle[channel]];
        sample += velocityGain * unisonGain * localMipMap[voiceData.unisonData[unisonNumber].lowerCurrentAngle[channel]];

        voiceData.unisonData[unisonNumber].upperCurrentAngle[channel] += voiceData.unisonData[unisonNumber].upperAngleDelta;
        voiceData.unisonData[unisonNumber].lowerCurrentAngle[channel] += voiceData.unisonData[unisonNumber].lowerAngleDelta;

        return sample;
    }

    void AdditiveVoice::updatePhases()
    {
        unisonPairCount = synthParameters.unisonCount->load();
        for (int channel = 0; channel < 2; channel++)
        {
            voiceData.currentAngle[channel] = getRandomPhase() + ( ( synthParameters.globalPhase->load() / 100 ) * juce::MathConstants<float>::twoPi );
            for (int unison = 0; unison < unisonPairCount; unison++)
            {
                voiceData.unisonData[unison].upperCurrentAngle[channel] = getRandomPhase() + ( ( synthParameters.globalPhase->load() / 100 ) * juce::MathConstants<float>::twoPi );
                voiceData.unisonData[unison].lowerCurrentAngle[channel] = getRandomPhase() + ( ( synthParameters.globalPhase->load() / 100 ) * juce::MathConstants<float>::twoPi );
            }
        }       
    }

    const float AdditiveVoice::getRandomPhase()
    {
        return ((rng.nextFloat() * synthParameters.randomPhaseRange->load() / 100) * juce::MathConstants<float>::twoPi);
    }

    void AdditiveVoice::updateFrequencies()
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

        for (int unison = 0; unison < unisonPairCount; unison++)
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

    void AdditiveVoice::updateAngles()
    {
        auto sampleRate = getSampleRate();
        float cyclesPerSample = voiceData.frequency / sampleRate;
        voiceData.angleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;

        unisonPairCount = synthParameters.unisonCount->load();
        for (int unison = 0; unison < unisonPairCount; unison++)
        {
            cyclesPerSample = (voiceData.frequency * voiceData.unisonData[unison].upperFrequencyOffset) / sampleRate;
            voiceData.unisonData[unison].upperAngleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;

            cyclesPerSample = (voiceData.frequency * voiceData.unisonData[unison].lowerFrequencyOffset) / sampleRate;
            voiceData.unisonData[unison].lowerAngleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;
        }
    }

    void AdditiveVoice::findMipMapToUse()
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

    void AdditiveVoice::resetProperties()
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

    void AdditiveVoice::updateADSRParams()
    {
        juce::ADSR::Parameters params;

        amplitudeADSR.setSampleRate(getSampleRate());

        params.attack = synthParameters.amplitudeADSRAttack->load() / 1000;
        params.decay = synthParameters.amplitudeADSRDecay->load() / 1000;
        params.sustain = synthParameters.amplitudeADSRSustain->load() / 100;
        params.release = synthParameters.amplitudeADSRRelease->load() / 1000;

        amplitudeADSR.setParameters(params);
    }
}