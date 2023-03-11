/*
  ==============================================================================

    SynthParameters.h
    Created: 11 Mar 2023 2:19:15am
    Author:  Habama10

  ==============================================================================
*/

#pragma once

constexpr int HARMONIC_N = 100;
constexpr int SYNTH_MAX_VOICES = 64;

struct SynthParameters
{
    int octaveTuning = 0;
    int semitoneTuning = 0;
    int fineTuningCents = 0;
    float partialGain[HARMONIC_N];
    float partialPhase[HARMONIC_N];
    float globalPhseStart = 0;
    float randomPhaseRange = 0;
    int unisonPairCount = 0;
    int unisonDetune = 0;
    float unisonGain = 0;
};