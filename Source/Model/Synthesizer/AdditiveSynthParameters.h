/*
==============================================================================

    AdditiveSynthParameters.h
    Created: 11 Mar 2023 2:19:15am
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "OscillatorParameters.h"

namespace Processor::Synthesizer
{
    constexpr int SYNTH_MAX_VOICES = 32;                    //The number of voices the synth can handle simultaneously

    struct AdditiveSynthParameters : public juce::AudioProcessorValueTreeState::Listener
    {
    public:
        AdditiveSynthParameters(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
        {
            linkParameters();
            registerListener(this);
        }

        ~AdditiveSynthParameters() override
        {
            removeListener(this);
        }

        void registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const
        {
            auto paramLayoutSchema = createParameterLayout();
            auto params = paramLayoutSchema->getParameters(false);

            for(auto param : params)
            {
                auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
                apvts.addParameterListener(id, listener);
            }
        }

        void removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const
        {
            auto paramLayoutSchema = createParameterLayout();
            auto params = paramLayoutSchema->getParameters(false);

            for(auto param : params)
            {
                auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
                apvts.removeParameterListener(id, listener);
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

            juce::AudioParameterFloatAttributes attr;

            //Gain for the oscillator. All voices are affected by this value
            auto synthGain = std::make_unique<juce::AudioParameterFloat>(
                "synthGain",
                "Gain",
                juce::NormalisableRange<float>(0.f, 100.f, 0.1, 0.4), 
                30.f,
                attr.withStringFromValueFunction([] (float value, int maximumStringLength)
                    {
                        juce::String string = juce::String(value, 1) + "%, " + juce::String(juce::Decibels::gainToDecibels(value/100), 1) + " dB";
                        return string;
                    }));
            synthGroup.get()->addChild(std::move(synthGain));

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

        const std::atomic<float>* synthGain;

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

        void parameterChanged(const juce::String &parameterID, float newValue) override
        {
            paramMap[parameterID] = newValue;
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

            synthGain = &paramMap["synthGain"];
            oscillatorOctaves = &paramMap["oscillatorOctaves"];
            oscillatorSemitones = &paramMap["oscillatorSemitones"];
            oscillatorFine = &paramMap["oscillatorFine"];
            pitchWheelRange = &paramMap["pitchWheelRange"];
            globalPhase = &paramMap["globalPhase"];
            randomPhaseRange = &paramMap["randomPhaseRange"];
            unisonCount = &paramMap["unisonCount"];
            unisonDetune = &paramMap["unisonDetune"];
            unisonGain = &paramMap["unisonGain"];
            amplitudeADSRAttack = &paramMap["amplitudeADSRAttack"];
            amplitudeADSRDecay = &paramMap["amplitudeADSRDecay"];
            amplitudeADSRSustain = &paramMap["amplitudeADSRSustain"];
            amplitudeADSRRelease = &paramMap["amplitudeADSRRelease"];
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthParameters)
    };
}
