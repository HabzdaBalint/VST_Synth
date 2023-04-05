/*
==============================================================================

    AdditiveSynthesizer.cpp
    Created: 25 Mar 2023 9:23:43pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "AdditiveSynthesizer.h"

AdditiveSynthesizer::AdditiveSynthesizer(juce::AudioProcessorValueTreeState& apvts) :
                        AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo())),
                        oscParameters(std::make_unique<OscillatorParameters>(apvts)),
                        synthParameters(std::make_unique<AdditiveSynthParameters>(apvts))
{
    for (size_t i = 0; i < LOOKUP_SIZE; i++)
    {
        mipMap.add(new juce::dsp::LookupTableTransform<float>());
    }

    synth->addSound(new AdditiveSound());

    for (size_t i = 0; i < SYNTH_MAX_VOICES; i++)
    {
        synth->addVoice(new AdditiveVoice(*synthParameters, mipMap));
    }
    synth->setNoteStealingEnabled(true);

    oscParameters->registerListener(this);
}

AdditiveSynthesizer::~AdditiveSynthesizer()
{
    synth->clearVoices();
    synth->clearSounds();
    lutUpdater.stopThread(10);
}

void AdditiveSynthesizer::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock)
{
    synth->setCurrentPlaybackSampleRate(sampleRate);
    for (size_t i = 0; i < synth->getNumVoices(); i++)
    {
        if (auto voice = dynamic_cast<AdditiveVoice *>(synth->getVoice(i)))
        {
            voice->setCurrentPlaybackSampleRate(sampleRate);
        }
    }

    juce::dsp::ProcessSpec processSpec;
    processSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
    processSpec.numChannels = getTotalNumOutputChannels();
    processSpec.sampleRate = sampleRate;
    synthGain->prepare(processSpec);
}

void AdditiveSynthesizer::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    synth->renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> audioBlock { buffer };

    synthGain->setGainLinear(synthParameters->synthGain->load() / 100);

    synthGain->process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
}

void AdditiveSynthesizer::parameterChanged(const juce::String &parameterID, float newValue)
{
    //start an update for the lookup table if necessary
    if( !lutUpdater.isThreadRunning() )
        lutUpdater.startThread();
    else
        missedUpdate = true;
}

void AdditiveSynthesizer::timerCallback()
{
    if( !lutUpdater.isThreadRunning() && missedUpdate )
    {
        lutUpdater.startThread();
        missedUpdate = false;
    }
}


void AdditiveSynthesizer::updateLookupTable()
{
    float peakAmplitude = getPeakAmplitude();

    if(peakAmplitude > 0.f)
    {
        float gainToNormalize = 1.f / peakAmplitude;
        for (size_t i = 0; i < LOOKUP_SIZE; i++)    //Generating peak-normalized lookup table
        {
            mipMap[i]->initialise(
                [this, i, gainToNormalize] (float x) { return gainToNormalize * oscParameters->getSample( x, std::floor( HARMONIC_N / pow(2, i) ) ); },
                0, juce::MathConstants<float>::twoPi,
                LOOKUP_POINTS);
        }
    }
    else
    {
        for (size_t i = 0; i < LOOKUP_SIZE; i++)
        {
            mipMap[i]->initialise(
                [] (float x) { return 0; },
                0, juce::MathConstants<float>::twoPi,
                LOOKUP_POINTS);
        }
    }
}

const float AdditiveSynthesizer::getPeakAmplitude()
{
    float peakAmplitude = 0.f;
    float gainToNormalize = 1.f;
    for (size_t i = 0; i < LOOKUP_POINTS; i++)  //Finding the peak amplitude of the lut
    {
        float sample = oscParameters->getSample( juce::jmap( (float)i, 0.f, LOOKUP_POINTS-1.f, 0.f, juce::MathConstants<float>::twoPi), HARMONIC_N);

        if( std::fabs(sample) > peakAmplitude)
        {
            peakAmplitude = std::fabs(sample);
        }
    }

    return peakAmplitude;
}