/*
==============================================================================

    SynthGain.h
    Created: 31 Mar 2023 8:26:31pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "LevelMeterGrid.h"
#include "GainKnob.h"

class SynthGainComponent : public juce::Component
{
public:
    SynthGainComponent(VST_SynthAudioProcessor& p) : audioProcessor(p) {}

    ~SynthGainComponent() override {}

    void paint(juce::Graphics& g) override
    {
    }

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;

        juce::Grid gainComponentGrid;
        gainComponentGrid.templateRows = { TrackInfo( Fr( 1 ) ) };
        gainComponentGrid.templateColumns = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 3 ) ) };
        gainComponentGrid.items = { juce::GridItem(*gainKnob) , juce::GridItem(*levelMeters) };
    }
    
private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<LevelMeterGrid> levelMeters = std::make_unique<LevelMeterGrid>(audioProcessor);
    std::unique_ptr<GainKnob> gainKnob = std::make_unique<GainKnob>(audioProcessor);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthGainComponent)
};