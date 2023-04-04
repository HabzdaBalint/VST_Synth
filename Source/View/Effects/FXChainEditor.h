/*
==============================================================================

    FXChainEditor.h
    Created: 18 Mar 2023 8:33:31pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"



class FXChainEditor : public juce::Component
{
public:
    FXChainEditor(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {

    }

    ~FXChainEditor() override {}

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXChainEditor)
};