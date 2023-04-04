/*
==============================================================================

    AdditiveSynthesizer.cpp
    Created: 25 Mar 2023 9:23:43pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "AdditiveSynthesizer.h"
#include "AdditiveSound.h"
#include "AdditiveVoice.h"

AdditiveSynthesizer::AdditiveSynthesizer(juce::AudioProcessorValueTreeState& apvts) :
                        AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo())),
                        synthParameters(std::make_unique<AdditiveSynthParameters>(apvts, lutUpdater))
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

void AdditiveSynthesizer::updateLookupTable()
{
    float gainToNormalize = findGainToNormalize();

    for (size_t i = 0; i < LOOKUP_SIZE; i++)    //Generating peak-normalized lookup table
    {
        mipMap[i]->initialise(
            [this, i, gainToNormalize] (float x) { return gainToNormalize * waveTableFormula( x, std::floor( HARMONIC_N / pow(2, i) ) ); },
            0, juce::MathConstants<float>::twoPi,
            LOOKUP_POINTS);
    }
}

const float AdditiveSynthesizer::findGainToNormalize()
{
    float peakAmplitude = 0.f;
    float gainToNormalize = 1.f;
    for (size_t i = 0; i < LOOKUP_POINTS; i++)  //Finding the peak amplitude of the waveform
    {
        float sample = waveTableFormula( juce::jmap( (float)i, 0.f, LOOKUP_POINTS-1.f, 0.f, juce::MathConstants<float>::twoPi), HARMONIC_N);

        if( std::fabs(sample) > peakAmplitude)
        {
            peakAmplitude = std::fabs(sample);
        }
    }

    if(peakAmplitude != 0.f)
    {
        gainToNormalize = 1.f / peakAmplitude;
    }

    return gainToNormalize;
}

const float AdditiveSynthesizer::waveTableFormula(float angle, int harmonics)
{
    float sample = 0.f;

    for (size_t i = 0; i < harmonics; i++)
    {
        float gain = synthParameters->partialGains[i]->load() / 100;
        float phase = synthParameters->partialPhases[i]->load() / 100;
        if (gain != 0.f)
        {
            sample += gain * sin((i + 1) * angle + phase * juce::MathConstants<float>::twoPi);
        }
    }

    return sample;
}