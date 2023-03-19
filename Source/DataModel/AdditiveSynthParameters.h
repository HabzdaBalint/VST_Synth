/*
==============================================================================

    AdditiveSynthParameters.h
    Created: 11 Mar 2023 2:19:15am
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

constexpr int SYNTH_MAX_VOICES = 64;
constexpr int HARMONIC_N = 64;
const int LOOKUP_SIZE = ceil(log2(HARMONIC_N) + 1);

struct AdditiveSynthParameters : juce::AudioProcessorValueTreeState::Listener
{
    AdditiveSynthParameters(std::function<void()> update) : update(update) {}

    juce::AudioParameterFloat *synthGain = nullptr;
    juce::AudioParameterFloat *octaveTuning = nullptr;
    juce::AudioParameterFloat *semitoneTuning = nullptr;
    juce::AudioParameterFloat *fineTuningCents = nullptr;
    juce::AudioParameterFloat *partialGain[HARMONIC_N] = {};
    juce::AudioParameterFloat *partialPhase[HARMONIC_N] = {};
    juce::AudioParameterFloat *globalPhseStart = nullptr;
    juce::AudioParameterFloat *randomPhaseRange = nullptr;
    juce::AudioParameterFloat *unisonPairCount = nullptr;
    juce::AudioParameterFloat *unisonDetune = nullptr;
    juce::AudioParameterFloat *unisonGain = nullptr;
    juce::AudioParameterFloat *pitchWheelRange = nullptr;
    juce::AudioParameterFloat *attack = nullptr;
    juce::AudioParameterFloat *decay = nullptr;
    juce::AudioParameterFloat *sustain = nullptr;
    juce::AudioParameterFloat *release = nullptr;

    std::function<void()> update = nullptr;

    /// @brief Creates and adds the synthesizer's parameters to the passed parameter layout
    /// @param layout Reference to a parameter layout
    void createParameterLayout(juce::AudioProcessorValueTreeState::ParameterLayout &layout)
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> vector;

        //---------------------// SYNTH PARAMS //---------------------//
        /*Gain for the oscillator. All voices are affected by this value*/
        synthGain = new juce::AudioParameterFloat("synthGain",
                                                  "Gain",
                                                  juce::NormalisableRange<float>(-90.f, 0.f, 0.1), -12.f);
        vector.emplace_back(synthGain);

        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            juce::String namePrefix = "Partial " + juce::String(i + 1) + " ";

            // Generating parameters to represent the linear gain values of the partials
            partialGain[i] = new juce::AudioParameterFloat(getPartialGainParameterName(i),
                                                           namePrefix + "Gain",
                                                           juce::NormalisableRange<float>(0.f, 1.f, 0.001), 0.f);
            vector.emplace_back(partialGain[i]);

            // Generating parameters to represent the phase of the partials. These are represented as multiples of 2*pi
            partialPhase[i] = new juce::AudioParameterFloat(getPartialPhaseParameterName(i),
                                                            namePrefix + "Phase",
                                                            juce::NormalisableRange<float>(0.f, 1.f, 0.01), 0.f);
            vector.emplace_back(partialPhase[i]);
        }

        /*Tuning of the generated notes in octaves*/
        octaveTuning = new juce::AudioParameterFloat("oscillatorOctaves",
                                                     "Octaves",
                                                     juce::NormalisableRange<float>(-2.f, 2.f, 1.f), 0.f);
        vector.emplace_back(octaveTuning);

        /*Tuning of the generated notes in semitones*/
        semitoneTuning = new juce::AudioParameterFloat("oscillatorSemitones",
                                                       "Semitones",
                                                       juce::NormalisableRange<float>(-12.f, 12.f, 1.f), 0.f);
        vector.emplace_back(semitoneTuning);

        /*Tuning of the generated notes in cents*/
        fineTuningCents = new juce::AudioParameterFloat("oscillatorFine",
                                                        "Fine Tuning",
                                                        juce::NormalisableRange<float>(-100.f, 100.f, 1.f), 0.f);
        vector.emplace_back(fineTuningCents);

        /*Pitch Wheel range in semitones*/
        pitchWheelRange = new juce::AudioParameterFloat("pitchWheelRange",
                                                        "Pitch Wheel Semitones",
                                                        juce::NormalisableRange<float>(0.f, 12.f, 1.f), 2.f);
        vector.emplace_back(pitchWheelRange);

        /*The global starting point of on waveform. in multiples of 2*pi*/
        globalPhseStart = new juce::AudioParameterFloat("globalPhase",
                                                        "Phase",
                                                        juce::NormalisableRange<float>(0.f, 1.f, 0.01), 0.f);
        vector.emplace_back(globalPhseStart);

        /*Sets the angle range for phase start randomization on new voices and unison. multiple of 2*pi*/
        randomPhaseRange = new juce::AudioParameterFloat("globalPhaseRNG",
                                                         "Phase Randomness",
                                                         juce::NormalisableRange<float>(0.f, 1.f, 0.01), 0.f);
        vector.emplace_back(randomPhaseRange);

        /*Pairs of unison to add (one tuned higher and one lower)*/
        unisonPairCount = new juce::AudioParameterFloat("unisonCount",
                                                        "Unison Count",
                                                        juce::NormalisableRange<float>(0.f, 5.f, 1.f), 0.f);
        vector.emplace_back(unisonPairCount);

        /*Detuning of the farthest unison pair in cents. The pairs inbetween have a tuning that is evenly distributed between the normal frequency and this one*/
        unisonDetune = new juce::AudioParameterFloat("unisonDetune",
                                                     "Unison Detune",
                                                     juce::NormalisableRange<float>(0.f, 100.f, 1.f), 0.f);
        vector.emplace_back(unisonDetune);

        /*Level of the unison in linear amplitude*/
        unisonGain = new juce::AudioParameterFloat("unisonGain",
                                                   "Unison Gain",
                                                   juce::NormalisableRange<float>(0.f, 1.f, 0.001), 0.f);
        vector.emplace_back(unisonGain);

        /*Attack time for the oscillator's amplitudes in ms*/
        attack = new juce::AudioParameterFloat("amplitudeADSRAttack",
                                               "A",
                                               juce::NormalisableRange<float>(0.f, 16000.f, 0.1), 0.5);
        vector.emplace_back(attack);

        /*Decay time for the oscillator's amplitudes in ms*/
        decay = new juce::AudioParameterFloat("amplitudeADSRDecay",
                                              "D",
                                              juce::NormalisableRange<float>(0.f, 16000.f, 0.1), 1000.f);
        vector.emplace_back(decay);

        /*Sustain level for the oscillator's amplitudes in linear amplitude*/
        sustain = new juce::AudioParameterFloat("amplitudeADSRSustain",
                                                "S",
                                                juce::NormalisableRange<float>(0.f, 1.f, 0.001), 1.f);
        vector.emplace_back(sustain);

        /*Release time for the oscillator's amplitudes in ms*/
        release = new juce::AudioParameterFloat("amplitudeADSRRelease",
                                                "R",
                                                juce::NormalisableRange<float>(0.f, 16000.f, 0.1), 50.f);
        vector.emplace_back(release);

        layout.add(vector.begin(), vector.end());
    }

    /// @brief Used for making the parameter ids of the the partials' gain parameters consistent
    /// @param index The index of the harmonic
    /// @return A consistent parameter id
    juce::String getPartialGainParameterName(size_t index)
    {
        return "partial" + juce::String(index) + "gain";
    }

    /// @brief Used for making the parameter ids of the the partials' phase parameters consistent
    /// @param index The index of the harmonic
    /// @return A consistent parameter id
    juce::String getPartialPhaseParameterName(size_t index)
    {
        return "partial" + juce::String(index) + "phase";
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        update();
    }
};

struct AdditiveSynthParametersAtomic
{
    std::atomic<float> octaveTuning = 0;
    std::atomic<float> semitoneTuning = 0;
    std::atomic<float> fineTuningCents = 0;
    std::atomic<float> partialGain[HARMONIC_N] = {};
    std::atomic<float> partialPhase[HARMONIC_N] = {};
    std::atomic<float> globalPhseStart = 0;
    std::atomic<float> randomPhaseRange = 0;
    std::atomic<float> unisonPairCount = 0;
    std::atomic<float> unisonDetune = 0;
    std::atomic<float> unisonGain = 0;
    std::atomic<float> pitchWheelRange = 0;
    std::atomic<float> attack = 0;
    std::atomic<float> decay = 0;
    std::atomic<float> sustain = 0;
    std::atomic<float> release = 0;
};