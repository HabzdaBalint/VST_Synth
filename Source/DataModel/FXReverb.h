/*
==============================================================================

    FXReverb.h
    Created: 14 Mar 2023 6:11:00pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "FXProcessorBase.h"
#include "FXReverbParameters.h"

using Reverb = juce::dsp::Reverb;

class FXReverb : public FXProcessorBase
{
public:
    FXReverb() {}
    ~FXReverb() {}
private:
    Reverb reverb;
};