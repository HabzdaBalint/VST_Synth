/*
==============================================================================

    SynthEditor.h
    Created: 30 Mar 2023 11:41:33pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "SynthGainComponent.h"

class SynthEditor : public juce::Component
{
public:
    SynthEditor(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        addAndMakeVisible(*synthGainComponent);
    }

    ~SynthEditor() override
    {

    }

    void paint(juce::Graphics&) override
    {

    }

    void resized() override
    {
        synthGainComponent->setBounds(getLocalBounds());
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<SynthGainComponent> synthGainComponent = std::make_unique<SynthGainComponent>(audioProcessor);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthEditor)
};

/*
    WaveformSelector
    Tuning
    phase
    
    Unison
    ADSR
    SynthGain
*/


