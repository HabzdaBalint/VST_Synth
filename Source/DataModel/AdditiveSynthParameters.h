/*
==============================================================================

    AdditiveSynthParameters.h
    Created: 11 Mar 2023 2:19:15am
    Author:  Habama10

==============================================================================
*/

#define SYNTHPARAMS_INCLUDED

#pragma once

#include <JuceHeader.h>

constexpr int SYNTH_MAX_VOICES = 32;                    //The number of voices the synth can handle simultaneously
constexpr int HARMONIC_N = 256;                         //The number of harmonics the synth can generate
constexpr int LOOKUP_POINTS = HARMONIC_N * 32;          //The number of calculated points in the lookup table
const int LOOKUP_SIZE = ceil(log2(HARMONIC_N) + 1);     //The number of mipmaps that need to be generated to avoid aliasing at a given harmonic count

struct LUTUpdater : juce::Thread
{
    LUTUpdater(std::function<void()> func) :
        juce::Thread("LUT Updater"),
        func(std::move(func))
    {}

    void run() override
    {
        func();
    }
private:
    std::function<void()> func;
};

struct AdditiveSynthParameters : public juce::AudioProcessorValueTreeState::Listener,
                                 public juce::Timer
{
public:
    AdditiveSynthParameters(juce::AudioProcessorValueTreeState& apvts, LUTUpdater& lutUpdater) : apvts(apvts), lutUpdater(lutUpdater)
    {
        registerListeners();
        linkParameters();
        startTimerHz(20);
    }

    void registerListeners()
    {
        apvts.addParameterListener("synthGain", this);

        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            apvts.addParameterListener(AdditiveSynthParameters::getPartialGainParameterName(i), this);
            apvts.addParameterListener(AdditiveSynthParameters::getPartialPhaseParameterName(i), this);
        }
        apvts.addParameterListener("oscillatorOctaves", this);
        apvts.addParameterListener("oscillatorSemitones", this);
        apvts.addParameterListener("oscillatorFine", this);
        apvts.addParameterListener("pitchWheelRange", this);
        apvts.addParameterListener("globalPhase", this);
        apvts.addParameterListener("randomPhaseRange", this);
        apvts.addParameterListener("unisonCount", this);
        apvts.addParameterListener("unisonDetune", this);
        apvts.addParameterListener("unisonGain", this);
        apvts.addParameterListener("amplitudeADSRAttack", this);
        apvts.addParameterListener("amplitudeADSRDecay", this);
        apvts.addParameterListener("amplitudeADSRSustain", this);
        apvts.addParameterListener("amplitudeADSRRelease", this);
    }

    void linkParameters()
    {
        auto paramLayoutSchema = createParameterLayout();

        auto params = paramLayoutSchema->getParameters(false);
        for ( auto param : params )
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            auto value = dynamic_cast<juce::RangedAudioParameter*>(param)->getNormalisableRange().convertFrom0to1(param->getDefaultValue());
            paramMap.emplace(id, value);
        }

        synthGain = (&paramMap.at("synthGain"));
        oscillatorOctaves = (&paramMap.at("oscillatorOctaves"));
        oscillatorSemitones = (&paramMap.at("oscillatorOctaves"));
        oscillatorFine = (&paramMap.at("oscillatorFine"));
        pitchWheelRange = (&paramMap.at("pitchWheelRange"));
        globalPhase = (&paramMap.at("globalPhase"));
        randomPhaseRange = (&paramMap.at("randomPhaseRange"));
        unisonCount = (&paramMap.at("unisonCount"));
        unisonDetune = (&paramMap.at("unisonDetune"));
        unisonGain = (&paramMap.at("unisonGain"));
        amplitudeADSRAttack = (&paramMap.at("amplitudeADSRAttack"));
        amplitudeADSRDecay = (&paramMap.at("amplitudeADSRDecay"));
        amplitudeADSRSustain = (&paramMap.at("amplitudeADSRSustain"));
        amplitudeADSRRelease = (&paramMap.at("amplitudeADSRRelease"));
        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            partialGains[i] = (&paramMap.at(getPartialGainParameterName(i)));
            partialPhases[i] = (&paramMap.at(getPartialPhaseParameterName(i)));
        }
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {   //todo: make an object of atomic float references that after initialization refer to things that need quick access, like partial gains
        paramMap[parameterID] = newValue;

        //start an update for the lookup table if necessary
        if( parameterID.contains("partial") )
        {
            if( !lutUpdater.isThreadRunning() )
                lutUpdater.startThread();
            else
                missedUpdate = true;
        }
    }

    void timerCallback() override
    {
        if( !lutUpdater.isThreadRunning() && missedUpdate )
        {
            lutUpdater.startThread();
            missedUpdate = false;
        }
    }

    /// @brief Creates and adds the synthesizer's parameters into a parameter group
    /// @return unique pointer to the group
    static std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
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
                AdditiveSynthParameters::getPartialGainParameterName(i),
                namePrefix + "Gain",
                juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
                0.f,
                attr.withAutomatable(false).withMeta(true));
            synthGroup.get()->addChild(std::move(partialGain));

            // Generating parameters to represent the phase of the partials. These are represented as a percentage value of 2 * pi radians 
            auto partialPhase = std::make_unique<juce::AudioParameterFloat>(
                AdditiveSynthParameters::getPartialPhaseParameterName(i),
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

        /*
        auto params = synthGroup->getParameters(false);
        for ( auto param : params)
        {
            auto id = dynamic_cast<juce::AudioParameterFloat*>(param)->getParameterID();
            auto value = dynamic_cast<juce::AudioParameterFloat*>(param)->getNormalisableRange().convertFrom0to1(param->getDefaultValue());
            paramMap.emplace(id, value);
        }
        paramRefs = std::make_unique<AdditiveSynthParameterReferences>(paramMap);
        */

        return synthGroup;
    }

    /// @brief Used for making the parameter ids of the the partials' gain parameters consistent
    /// @param index The index of the harmonic
    /// @return A consistent parameter id
    static const juce::String getPartialGainParameterName(size_t index)
    {
        return "partial" + juce::String(index) + "gain";
    }

    /// @brief Used for making the parameter ids of the the partials' phase parameters consistent
    /// @param index The index of the harmonic
    /// @return A consistent parameter id
    static const juce::String getPartialPhaseParameterName(size_t index)
    {
        return "partial" + juce::String(index) + "phase";
    }

    const std::atomic<float>* synthGain;

    const std::atomic<float>* partialGains[HARMONIC_N];
    const std::atomic<float>* partialPhases[HARMONIC_N];

    const std::atomic<float>* oscillatorOctaves;
    const std::atomic<float>* oscillatorSemitones;
    const std::atomic<float>* oscillatorFine;
    const std::atomic<float>* pitchWheelRange;

    const std::atomic<float>* globalPhase;
    const std::atomic<float>* randomPhaseRange;

    const std::atomic<float>* unisonCount;
    const std::atomic<float>* unisonDetune;
    const std::atomic<float>* unisonGain;

    const std::atomic<float>* amplitudeADSRAttack;
    const std::atomic<float>* amplitudeADSRDecay;
    const std::atomic<float>* amplitudeADSRSustain;
    const std::atomic<float>* amplitudeADSRRelease;
private:
    juce::AudioProcessorValueTreeState& apvts;
    std::unordered_map<juce::String, std::atomic<float>> paramMap;
    LUTUpdater& lutUpdater;
    std::atomic<bool> missedUpdate = false;
};