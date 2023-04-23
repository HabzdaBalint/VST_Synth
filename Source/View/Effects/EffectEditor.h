/*
==============================================================================

    EffectEditor.h
    Created: 6 Apr 2023 4:46:50pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class EffectEditor : public juce::Component
{
public:
    EffectEditor() {}

    ~EffectEditor() override {}

    void paint(juce::Graphics& g) override = 0;

    void resized() override = 0;

    virtual const int getIdealHeight() = 0;
};