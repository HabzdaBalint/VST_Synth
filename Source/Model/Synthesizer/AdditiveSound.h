/*
==============================================================================

    AdditiveSound.h
    Created: 11 Mar 2023 2:13:10am
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Synthesizer
{
    class AdditiveSound : public juce::SynthesiserSound
    {
    public:
        AdditiveSound() {}
        ~AdditiveSound() {}
        bool appliesToNote(int midiNoteNumber) override { return true; }
        bool appliesToChannel(int midiChannelNumber) override { return true; }
    };
}
