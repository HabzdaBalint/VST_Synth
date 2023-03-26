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
constexpr int HARMONIC_N = 128;
constexpr int LOOKUP_POINTS = 2048;
const int LOOKUP_SIZE = ceil(log2(HARMONIC_N) + 1);

struct AdditiveSynthParameters : juce::AudioProcessorValueTreeState::Listener
{
    AdditiveSynthParameters(std::function<void()> update) : update(update) {}

    std::function<void()> update = nullptr;

    /// @brief Creates and adds the synthesizer's parameters into a parameter group
    /// @return unique pointer to the group
    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> synthGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("synthGroup", "Synthesizer", "|"));

        //---------------------// SYNTH PARAMS //---------------------//
        //Gain for the oscillator. All voices are affected by this value
        auto synthGain = std::make_unique<juce::AudioParameterFloat>("synthGain",
                                                  "Gain",
                                                  juce::NormalisableRange<float>(-90.f, 0.f, 0.1), -12.f);
        synthGroup.get()->addChild(std::move(synthGain));
        
        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            juce::String namePrefix = "Partial " + juce::String(i + 1) + " ";

            // Generating parameters to represent the linear gain values of the partials
            auto partialGain = std::make_unique<juce::AudioParameterFloat>(getPartialGainParameterName(i),
                                                           namePrefix + "Gain",
                                                           juce::NormalisableRange<float>(0.f, 1.f, 0.001), 0.f /*+ ( 1.f / ( i + 1.f ) ) */);
            synthGroup.get()->addChild(std::move(partialGain));

            // Generating parameters to represent the phase of the partials. These are represented as radians
            auto partialPhase = std::make_unique<juce::AudioParameterFloat>(getPartialPhaseParameterName(i),
                                                            namePrefix + "Phase",
                                                            juce::NormalisableRange<float>(0.f, 1.f, 0.01), 0.f);
            synthGroup.get()->addChild(std::move(partialPhase));
        }

        //Tuning of the generated notes in octaves
        auto octaveTuning = std::make_unique<juce::AudioParameterFloat>("oscillatorOctaves",
                                                     "Octaves",
                                                     juce::NormalisableRange<float>(-2.f, 2.f, 1.f), 0.f);
        synthGroup.get()->addChild(std::move(octaveTuning));

        //Tuning of the generated notes in semitones
        auto semitoneTuning = std::make_unique<juce::AudioParameterFloat>("oscillatorSemitones",
                                                       "Semitones",
                                                       juce::NormalisableRange<float>(-12.f, 12.f, 1.f), 0.f);
        synthGroup.get()->addChild(std::move(semitoneTuning));

        //Tuning of the generated notes in cents
        auto fineTuningCents = std::make_unique<juce::AudioParameterFloat>("oscillatorFine",
                                                        "Fine Tuning",
                                                        juce::NormalisableRange<float>(-100.f, 100.f, 1.f), 0.f);
        synthGroup.get()->addChild(std::move(fineTuningCents));

        //Pitch Wheel range in semitones
        auto pitchWheelRange = std::make_unique<juce::AudioParameterFloat>("pitchWheelRange",
                                                        "Pitch Wheel Semitones",
                                                        juce::NormalisableRange<float>(0.f, 12.f, 1.f), 2.f);
        synthGroup.get()->addChild(std::move(pitchWheelRange));

        //The global starting point of on waveform. in radians
        auto globalPhseStart = std::make_unique<juce::AudioParameterFloat>("globalPhase",
                                                        "Phase",
                                                        juce::NormalisableRange<float>(0.f, 1.f, 0.01), 0.f);
        synthGroup.get()->addChild(std::move(globalPhseStart));

        //Sets the angle range for phase start randomization on new voices and unison. in radians
        auto randomPhaseRange = std::make_unique<juce::AudioParameterFloat>("globalPhaseRNG",
                                                         "Phase Randomness",
                                                         juce::NormalisableRange<float>(0.f, 1.f, 0.01), 0.f);
        synthGroup.get()->addChild(std::move(randomPhaseRange));

        //Pairs of unison to add (one tuned higher and one lower)
        auto unisonPairCount = std::make_unique<juce::AudioParameterFloat>("unisonCount",
                                                        "Unison Count",
                                                        juce::NormalisableRange<float>(0.f, 5.f, 1.f), 0.f);
        synthGroup.get()->addChild(std::move(unisonPairCount));

        //Detuning of the farthest unison pair in cents. The pairs inbetween have a tuning that is evenly distributed between the normal frequency and this one
        auto unisonDetune = std::make_unique<juce::AudioParameterFloat>("unisonDetune",
                                                     "Unison Detune",
                                                     juce::NormalisableRange<float>(0.f, 100.f, 1.f), 0.f);
        synthGroup.get()->addChild(std::move(unisonDetune));

        //Level of the unison in linear amplitude
        auto unisonGain = std::make_unique<juce::AudioParameterFloat>("unisonGain",
                                                   "Unison Gain",
                                                   juce::NormalisableRange<float>(0.f, 1.f, 0.001), 0.f);
        synthGroup.get()->addChild(std::move(unisonGain));

        //Attack time for the oscillator's amplitudes in ms
        auto attack = std::make_unique<juce::AudioParameterFloat>("amplitudeADSRAttack",
                                               "A",
                                               juce::NormalisableRange<float>(0.f, 16000.f, 0.1), 0.5);
        synthGroup.get()->addChild(std::move(attack));

        //Decay time for the oscillator's amplitudes in ms
        auto decay = std::make_unique<juce::AudioParameterFloat>("amplitudeADSRDecay",
                                              "D",
                                              juce::NormalisableRange<float>(0.f, 16000.f, 0.1), 1000.f);
        synthGroup.get()->addChild(std::move(decay));

        //Sustain level for the oscillator's amplitudes in linear amplitude
        auto sustain = std::make_unique<juce::AudioParameterFloat>("amplitudeADSRSustain",
                                                "S",
                                                juce::NormalisableRange<float>(0.f, 1.f, 0.001), 1.f);
        synthGroup.get()->addChild(std::move(sustain));

        //Release time for the oscillator's amplitudes in ms
        auto release = std::make_unique<juce::AudioParameterFloat>("amplitudeADSRRelease",
                                                "R",
                                                juce::NormalisableRange<float>(0.f, 16000.f, 0.1), 50.f);
        synthGroup.get()->addChild(std::move(release));

        return synthGroup;
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