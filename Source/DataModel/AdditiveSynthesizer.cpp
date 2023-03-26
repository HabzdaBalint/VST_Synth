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

AdditiveSynthesizer::AdditiveSynthesizer() : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo()))
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

const juce::String AdditiveSynthesizer::getName() const { return "Additive Synthesizer"; }

bool AdditiveSynthesizer::acceptsMidi() const { return true; }
bool AdditiveSynthesizer::producesMidi() const { return false; }

juce::AudioProcessorEditor* AdditiveSynthesizer::createEditor()
{
    return new AdditiveSynthesizerEditor(*this);
}

bool AdditiveSynthesizer::hasEditor() const { return true; }

int AdditiveSynthesizer::getNumPrograms() { return 1; }
int AdditiveSynthesizer::getCurrentProgram() { return 0; }
void AdditiveSynthesizer::setCurrentProgram(int) {}
const juce::String AdditiveSynthesizer::getProgramName(int) { return {}; }
void AdditiveSynthesizer::changeProgramName(int, const juce::String &) {}

void AdditiveSynthesizer::getStateInformation(juce::MemoryBlock& destData) {}
void AdditiveSynthesizer::setStateInformation(const void* data, int sizeInBytes) {}
double AdditiveSynthesizer::getTailLengthSeconds() const { return 0; }

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

void AdditiveSynthesizer::updateSynthParameters()
{
    synthGain->setGainDecibels(synthParameters.synthGain->get());
    synthParametersAtomic.octaveTuning = synthParameters.octaveTuning->get();
    synthParametersAtomic.semitoneTuning = synthParameters.semitoneTuning->get();
    synthParametersAtomic.fineTuningCents = synthParameters.fineTuningCents->get();

    synthParametersAtomic.pitchWheelRange = synthParameters.pitchWheelRange->get();

    synthParametersAtomic.globalPhseStart = synthParameters.globalPhseStart->get();
    synthParametersAtomic.randomPhaseRange = synthParameters.randomPhaseRange->get();

    synthParametersAtomic.unisonPairCount = synthParameters.unisonPairCount->get();
    synthParametersAtomic.unisonGain = synthParameters.unisonGain->get();
    synthParametersAtomic.unisonDetune = synthParameters.unisonDetune->get();

    synthParametersAtomic.attack = synthParameters.attack->get() / 1000;
    synthParametersAtomic.decay = synthParameters.decay->get() / 1000;
    synthParametersAtomic.sustain = synthParameters.sustain->get();
    synthParametersAtomic.release = synthParameters.release->get() / 1000;

    for (size_t i = 0; i < HARMONIC_N; i++)
    {
        synthParametersAtomic.partialGain[i] = synthParameters.partialGain[i]->get();
        synthParametersAtomic.partialPhase[i] = synthParameters.partialPhase[i]->get();
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

void AdditiveSynthesizer::registerListeners(juce::AudioProcessorValueTreeState &apvts)
{
    apvts.addParameterListener("synthGain", &synthParameters);

    for (size_t i = 0; i < HARMONIC_N; i++)
    {
        apvts.addParameterListener(synthParameters.getPartialGainParameterName(i), &synthParameters);
        apvts.addParameterListener(synthParameters.getPartialPhaseParameterName(i), &synthParameters);
    }
    apvts.addParameterListener("oscillatorOctaves", &synthParameters);
    apvts.addParameterListener("oscillatorSemitones", &synthParameters);
    apvts.addParameterListener("oscillatorFine", &synthParameters);
    apvts.addParameterListener("pitchWheelRange", &synthParameters);
    apvts.addParameterListener("globalPhase", &synthParameters);
    apvts.addParameterListener("globalPhaseRNG", &synthParameters);
    apvts.addParameterListener("unisonCount", &synthParameters);
    apvts.addParameterListener("unisonDetune", &synthParameters);
    apvts.addParameterListener("unisonGain", &synthParameters);
    apvts.addParameterListener("amplitudeADSRAttack", &synthParameters);
    apvts.addParameterListener("amplitudeADSRDecay", &synthParameters);
    apvts.addParameterListener("amplitudeADSRSustain", &synthParameters);
    apvts.addParameterListener("amplitudeADSRRelease", &synthParameters);

    synthParameters.update();
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