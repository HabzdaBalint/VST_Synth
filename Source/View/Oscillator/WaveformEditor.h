/*
==============================================================================

    WaveformEditor.h
    Created: 31 Mar 2023 8:00:13pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"
#include "PartialSlider.h"

class WaveformEditor : public juce::Component
{
public:
    WaveformEditor(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            partialSliders.add(new PartialSlider(audioProcessor, i));
            addAndMakeVisible(partialSliders[i]);
        }
        
        auto bounds = getLocalBounds();
        bounds.setWidth(HARMONIC_N * WIDTH_PARTIAL_SLIDERS_PX);
        setBounds(bounds);
    }

    ~WaveformEditor() override {}

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
        auto bounds1 = getLocalBounds();

        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            auto bounds = getLocalBounds();
            bounds.setWidth(WIDTH_PARTIAL_SLIDERS_PX);
            bounds.setX(i * WIDTH_PARTIAL_SLIDERS_PX);
            partialSliders[i]->setBounds(bounds);
        }
    }
    
private:
    VST_SynthAudioProcessor& audioProcessor;

    juce::OwnedArray<PartialSlider> partialSliders;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformEditor)
};