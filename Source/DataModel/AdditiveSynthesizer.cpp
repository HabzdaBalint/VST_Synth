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
        synth->addVoice(new AdditiveVoice(synthParameters, mipMap));
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
    synthGain->process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
}

void AdditiveSynthesizer::parameterChanged(const juce::String &parameterID, float newValue)
{
    updateSynthParameters();

    //start an update for the lookup table if necessary
    if(parameterID.contains("partial") && !lutUpdater.isThreadRunning())
    {
        lutUpdater.startThread();
    }
}

void AdditiveSynthesizer::updateSynthParameters()
{
    synthGain->setGainLinear(apvts->getRawParameterValue("synthGain")->load() / 100);

    synthParameters.octaveTuning = apvts->getRawParameterValue("oscillatorOctaves")->load();
    synthParameters.semitoneTuning = apvts->getRawParameterValue("oscillatorSemitones")->load();
    synthParameters.fineTuningCents = apvts->getRawParameterValue("oscillatorFine")->load();

    synthParameters.pitchWheelRange = apvts->getRawParameterValue("pitchWheelRange")->load();

    synthParameters.globalPhseStart = apvts->getRawParameterValue("globalPhase")->load() / 100;
    synthParameters.randomPhaseRange = apvts->getRawParameterValue("randomPhaseRange")->load() / 100;

    synthParameters.unisonPairCount = apvts->getRawParameterValue("unisonCount")->load();
    synthParameters.unisonGain = apvts->getRawParameterValue("unisonGain")->load() / 100;
    synthParameters.unisonDetune = apvts->getRawParameterValue("unisonDetune")->load();

    synthParameters.attack = apvts->getRawParameterValue("amplitudeADSRAttack")->load() / 1000;
    synthParameters.decay = apvts->getRawParameterValue("amplitudeADSRDecay")->load() / 1000;
    synthParameters.sustain = apvts->getRawParameterValue("amplitudeADSRSustain")->load() / 100;
    synthParameters.release = apvts->getRawParameterValue("amplitudeADSRRelease")->load() / 1000;

    for (size_t i = 0; i < HARMONIC_N; i++)
    {
        synthParameters.partialGain[i] = apvts->getRawParameterValue(getPartialGainParameterName(i))->load() / 100;
        synthParameters.partialPhase[i] = apvts->getRawParameterValue(getPartialPhaseParameterName(i))->load() / 100;
    }
}

void AdditiveSynthesizer::updateLookupTable()
{
    float peakAmplitude = 0.f;
    float gainToNormalize = 1.f;
    for (size_t i = 0; i < LOOKUP_POINTS; i++)  //Finding the peak amplitude of the waveform
    {
        float sample = WaveTableFormula( juce::jmap( (float)i, 0.f, LOOKUP_POINTS-1.f, 0.f, juce::MathConstants<float>::twoPi), HARMONIC_N);

        if( std::fabs(sample) > peakAmplitude)
        {
            peakAmplitude = std::fabs(sample);
        }
    }

    if(peakAmplitude != 0.f)
    {
        gainToNormalize = 1.f / peakAmplitude;
    }

    for (size_t i = 0; i < LOOKUP_SIZE; i++)    //Generating peak-normalized lookup table
    {
        mipMap[i]->initialise(
            [this, i, gainToNormalize] (float x) { return gainToNormalize * WaveTableFormula( x, std::floor( HARMONIC_N / pow(2, i) ) ); },
            0, juce::MathConstants<float>::twoPi,
            LOOKUP_POINTS);
    }
}

const float AdditiveSynthesizer::WaveTableFormula(float angle, int harmonics)
{
    float sample = 0.f;

    for (size_t i = 0; i < harmonics; i++)
    {
        if (synthParameters.partialGain[i] != 0.f)
        {
            sample += synthParameters.partialGain[i] * sin((i + 1) * angle + synthParameters.partialPhase[i] * juce::MathConstants<float>::twoPi);
        }
    }

    return sample;
}

void AdditiveSynthesizer::connectApvts(juce::AudioProcessorValueTreeState& apvts)
{
    this->apvts = &apvts;

    registerListeners();
}

void AdditiveSynthesizer::registerListeners()
{
    apvts->addParameterListener("synthGain", this);

    for (size_t i = 0; i < HARMONIC_N; i++)
    {
        apvts->addParameterListener(getPartialGainParameterName(i), this);
        apvts->addParameterListener(getPartialPhaseParameterName(i), this);
    }
    apvts->addParameterListener("oscillatorOctaves", this);
    apvts->addParameterListener("oscillatorSemitones", this);
    apvts->addParameterListener("oscillatorFine", this);
    apvts->addParameterListener("pitchWheelRange", this);
    apvts->addParameterListener("globalPhase", this);
    apvts->addParameterListener("randomPhaseRange", this);
    apvts->addParameterListener("unisonCount", this);
    apvts->addParameterListener("unisonDetune", this);
    apvts->addParameterListener("unisonGain", this);
    apvts->addParameterListener("amplitudeADSRAttack", this);
    apvts->addParameterListener("amplitudeADSRDecay", this);
    apvts->addParameterListener("amplitudeADSRSustain", this);
    apvts->addParameterListener("amplitudeADSRRelease", this);
}

std::unique_ptr<juce::AudioProcessorParameterGroup> AdditiveSynthesizer::createParameterLayout()
{
    std::unique_ptr<juce::AudioProcessorParameterGroup> synthGroup (
        std::make_unique<juce::AudioProcessorParameterGroup>(
            "synthGroup", 
            "Synthesizer", 
            "|"));

    //Gain for the oscillator. All voices are affected by this value
    auto synthGain = std::make_unique<juce::AudioParameterFloat>(
        "synthGain",
        "Gain",
        juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
        70.f);
    synthGroup.get()->addChild(std::move(synthGain));
    
    juce::AudioParameterFloatAttributes attr;

    for (size_t i = 0; i < HARMONIC_N; i++)
    {
        juce::String namePrefix = "Partial " + juce::String(i + 1) + " ";

        // Generating parameters to represent the amplitude percentage values of the partials
        auto partialGain = std::make_unique<juce::AudioParameterFloat>(
            getPartialGainParameterName(i),
            namePrefix + "Gain",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            0.f,
            attr.withAutomatable(false).withMeta(true));
        synthGroup.get()->addChild(std::move(partialGain));

        // Generating parameters to represent the phase of the partials. These are represented as a percentage value of 2 * pi radians 
        auto partialPhase = std::make_unique<juce::AudioParameterFloat>(
            getPartialPhaseParameterName(i),
            namePrefix + "Phase",
            juce::NormalisableRange<float>(0.f, 99.f, 1.f), 
            0.f,
            attr.withAutomatable(false).withMeta(true));
        synthGroup.get()->addChild(std::move(partialPhase));
    }

    //Tuning of the generated notes in octaves
    auto octaveTuning = std::make_unique<juce::AudioParameterFloat>(
        "oscillatorOctaves",
        "Octave Tuning",
        juce::NormalisableRange<float>(-2.f, 2.f, 1.f), 
        0.f);
    synthGroup.get()->addChild(std::move(octaveTuning));

    //Tuning of the generated notes in semitones
    auto semitoneTuning = std::make_unique<juce::AudioParameterFloat>(
        "oscillatorSemitones",
        "Semitone Tuning",
        juce::NormalisableRange<float>(-12.f, 12.f, 1.f), 
        0.f);
    synthGroup.get()->addChild(std::move(semitoneTuning));

    //Tuning of the generated notes in cents
    auto fineTuningCents = std::make_unique<juce::AudioParameterFloat>(
        "oscillatorFine",
        "Fine Tuning",
        juce::NormalisableRange<float>(-100.f, 100.f, 1.f), 
        0.f);
    synthGroup.get()->addChild(std::move(fineTuningCents));

    //Pitch Wheel range in semitones
    auto pitchWheelRange = std::make_unique<juce::AudioParameterFloat>(
        "pitchWheelRange",
        "Pitch Wheel Range",
        juce::NormalisableRange<float>(0.f, 12.f, 1.f), 
        2.f);
    synthGroup.get()->addChild(std::move(pitchWheelRange));

    //The global starting point of on waveform. as a percentage value of 2 * pi radians
    auto globalPhseStart = std::make_unique<juce::AudioParameterFloat>(
        "globalPhase",
        "Phase",
        juce::NormalisableRange<float>(0.f, 99.9, 0.1), 
        0.f);
    synthGroup.get()->addChild(std::move(globalPhseStart));

    //Sets the maximum range for phase randomization on sounds and unison. as a percentage value of 2 * pi radians ( random range is [0..globalPhaseRNG] )
    auto randomPhaseRange = std::make_unique<juce::AudioParameterFloat>(
        "randomPhaseRange",
        "Phase Randomness",
        juce::NormalisableRange<float>(0.f, 99.9, 0.1), 
        0.f);
    synthGroup.get()->addChild(std::move(randomPhaseRange));

    //Pairs of unison to add (one tuned higher and one lower)
    auto unisonPairCount = std::make_unique<juce::AudioParameterFloat>(
        "unisonCount",
        "Unison Count",
        juce::NormalisableRange<float>(0.f, 5.f, 1.f), 
        0.f);
    synthGroup.get()->addChild(std::move(unisonPairCount));

    //Detuning of the farthest unison pair in cents. The pairs inbetween have a tuning that is evenly distributed between the normal frequency and this one
    auto unisonDetune = std::make_unique<juce::AudioParameterFloat>(
        "unisonDetune",
        "Unison Detune",
        juce::NormalisableRange<float>(0.f, 100.f, 0.5, 0.6), 
        15.f);
    synthGroup.get()->addChild(std::move(unisonDetune));

    //Level of the unison in linear amplitude
    auto unisonGain = std::make_unique<juce::AudioParameterFloat>(
        "unisonGain",
        "Unison Gain",
        juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
        50.f);
    synthGroup.get()->addChild(std::move(unisonGain));

    //Attack time for the oscillator's amplitudes in ms
    auto attack = std::make_unique<juce::AudioParameterFloat>(
        "amplitudeADSRAttack",
        "A",
        juce::NormalisableRange<float>(0.f, 10000.f, 0.1, 0.45), 
        5.f);
    synthGroup.get()->addChild(std::move(attack));

    //Decay time for the oscillator's amplitudes in ms
    auto decay = std::make_unique<juce::AudioParameterFloat>(
        "amplitudeADSRDecay",
        "D",
        juce::NormalisableRange<float>(0.f, 10000.f, 0.1, 0.45), 
        1000.f);
    synthGroup.get()->addChild(std::move(decay));

    //Sustain level for the oscillator's amplitudes in linear amplitude
    auto sustain = std::make_unique<juce::AudioParameterFloat>(
        "amplitudeADSRSustain",
        "S",
        juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
        100.f);
    synthGroup.get()->addChild(std::move(sustain));

    //Release time for the oscillator's amplitudes in ms
    auto release = std::make_unique<juce::AudioParameterFloat>(
        "amplitudeADSRRelease",
        "R",
        juce::NormalisableRange<float>(0.f, 10000.f, 0.1, 0.45), 
        50.f);
    synthGroup.get()->addChild(std::move(release));

    return synthGroup;
}

juce::String AdditiveSynthesizer::getPartialGainParameterName(size_t index)
{
    return "partial" + juce::String(index) + "gain";
}

juce::String AdditiveSynthesizer::getPartialPhaseParameterName(size_t index)
{
    return "partial" + juce::String(index) + "phase";
}