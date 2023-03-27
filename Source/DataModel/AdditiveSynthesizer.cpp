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
#include "../View/AdditiveSynthesizerEditor.h"

AdditiveSynthesizer::AdditiveSynthesizer()
                    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo()))
{
    for (size_t i = 0; i < LOOKUP_SIZE; i++)
    {
        mipMap.add(new juce::dsp::LookupTableTransform<float>());
    }

    synth->addSound(new AdditiveSound());

    for (size_t i = 0; i < SYNTH_MAX_VOICES; i++)
    {
        synth->addVoice(new AdditiveVoice(synthParametersAtomic, mipMap));
    }
    synth->setNoteStealingEnabled(true);
}

AdditiveSynthesizer::~AdditiveSynthesizer()
{
    for (size_t i = 0; i < mipMap.size(); i++)
    {
        delete (mipMap[i]);
    }
    mipMap.removeRange(0, mipMap.size());
    delete(synthGain);
    delete(synth);
}

juce::AudioProcessorEditor* AdditiveSynthesizer::createEditor()
{
    return new AdditiveSynthesizerEditor(*this, apvts);
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

void AdditiveSynthesizer::releaseResources() {}

void AdditiveSynthesizer::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    synth->renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> audioBlock{buffer};
    synthGain->process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
}

void AdditiveSynthesizer::connectApvts(juce::AudioProcessorValueTreeState& apvts)
{
    this->apvts = &apvts;

    registerListeners();
}

void AdditiveSynthesizer::updateSynthParameters()
{
    synthGain->setGainDecibels(apvts->getRawParameterValue("synthGain")->load());

    synthParametersAtomic.octaveTuning = apvts->getRawParameterValue("oscillatorOctaves")->load();
    synthParametersAtomic.semitoneTuning = apvts->getRawParameterValue("oscillatorSemitones")->load();
    synthParametersAtomic.fineTuningCents = apvts->getRawParameterValue("oscillatorFine")->load();

    synthParametersAtomic.pitchWheelRange = apvts->getRawParameterValue("pitchWheelRange")->load();

    synthParametersAtomic.globalPhseStart = apvts->getRawParameterValue("globalPhase")->load();
    synthParametersAtomic.randomPhaseRange = apvts->getRawParameterValue("globalPhaseRNG")->load();

    synthParametersAtomic.unisonPairCount = apvts->getRawParameterValue("unisonCount")->load();
    synthParametersAtomic.unisonGain = apvts->getRawParameterValue("unisonGain")->load();
    synthParametersAtomic.unisonDetune = apvts->getRawParameterValue("unisonDetune")->load();

    synthParametersAtomic.attack = apvts->getRawParameterValue("amplitudeADSRAttack")->load() / 1000;
    synthParametersAtomic.decay = apvts->getRawParameterValue("amplitudeADSRDecay")->load() / 1000;
    synthParametersAtomic.sustain = apvts->getRawParameterValue("amplitudeADSRSustain")->load();
    synthParametersAtomic.release = apvts->getRawParameterValue("amplitudeADSRRelease")->load() / 1000;

    for (size_t i = 0; i < HARMONIC_N; i++)
    {
        synthParametersAtomic.partialGain[i] = apvts->getRawParameterValue(synthParameters.getPartialGainParameterName(i))->load();
        synthParametersAtomic.partialPhase[i] = apvts->getRawParameterValue(synthParameters.getPartialPhaseParameterName(i))->load();
    }

    AdditiveSynthesizer::updateLookupTable();
}

void AdditiveSynthesizer::updateLookupTable()
{
    for (size_t i = 0; i < LOOKUP_SIZE; i++)
    {
        mipMap[i]->initialise([this, i](float x)
                                { return WaveTableFormula(x, std::floor(HARMONIC_N / pow(2, i))); },
                                0, juce::MathConstants<float>::twoPi, LOOKUP_POINTS);
    }
}

void AdditiveSynthesizer::registerListeners()
{
    apvts->addParameterListener("synthGain", &synthParameters);

    for (size_t i = 0; i < HARMONIC_N; i++)
    {
        apvts->addParameterListener(synthParameters.getPartialGainParameterName(i), &synthParameters);
        apvts->addParameterListener(synthParameters.getPartialPhaseParameterName(i), &synthParameters);
    }
    apvts->addParameterListener("oscillatorOctaves", &synthParameters);
    apvts->addParameterListener("oscillatorSemitones", &synthParameters);
    apvts->addParameterListener("oscillatorFine", &synthParameters);
    apvts->addParameterListener("pitchWheelRange", &synthParameters);
    apvts->addParameterListener("globalPhase", &synthParameters);
    apvts->addParameterListener("globalPhaseRNG", &synthParameters);
    apvts->addParameterListener("unisonCount", &synthParameters);
    apvts->addParameterListener("unisonDetune", &synthParameters);
    apvts->addParameterListener("unisonGain", &synthParameters);
    apvts->addParameterListener("amplitudeADSRAttack", &synthParameters);
    apvts->addParameterListener("amplitudeADSRDecay", &synthParameters);
    apvts->addParameterListener("amplitudeADSRSustain", &synthParameters);
    apvts->addParameterListener("amplitudeADSRRelease", &synthParameters);

    synthParameters.update();
}

void AdditiveSynthesizer::handleAsyncUpdate()
{
    AdditiveSynthesizer::updateSynthParameters();
}

const float AdditiveSynthesizer::WaveTableFormula(float angle, int harmonics)
{
    float sample = 0.f;

    for (size_t i = 0; i < harmonics; i++)
    {
        if (synthParametersAtomic.partialGain[i] != 0.f)
        {
            sample += synthParametersAtomic.partialGain[i] * sin((i + 1) * angle + synthParametersAtomic.partialPhase[i] * juce::MathConstants<float>::twoPi);
        }
    }

    return sample;
}