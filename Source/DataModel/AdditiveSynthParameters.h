/*
==============================================================================

    AdditiveSynthParameters.h
    Created: 11 Mar 2023 2:19:15am
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

constexpr int SYNTH_MAX_VOICES = 128;                   //The number of voices the synth can handle simultaneously
constexpr int HARMONIC_N = 256;                         //The number of harmonics the synth can generate
const int LOOKUP_POINTS = HARMONIC_N * 32;              //The number of calculated points in the lookup table
const int LOOKUP_SIZE = ceil(log2(HARMONIC_N) + 1);     //The number of mipmaps that need to be generated to avoid aliasing at a given harmonic count

struct AdditiveSynthParameters
{
    float octaveTuning = 0;
    float semitoneTuning = 0;
    float fineTuningCents = 0;
    std::atomic<float> partialGain[HARMONIC_N] = {};
    std::atomic<float> partialPhase[HARMONIC_N] = {};
    float globalPhseStart = 0;
    float randomPhaseRange = 0;
    float unisonPairCount = 0;
    float unisonDetune = 0;
    float unisonGain = 0;
    float pitchWheelRange = 0;
    float attack = 0;
    float decay = 0;
    float sustain = 0;
    float release = 0;
};