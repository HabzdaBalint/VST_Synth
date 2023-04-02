/*
==============================================================================

    LevelMeterGrid.h
    Created: 31 Mar 2023 9:19:30pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "LevelMeter.h"

class LevelMeterGrid : public juce::Component
{
public:
    LevelMeterGrid(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        auto numChannels = p.getTotalNumOutputChannels();
        for (size_t i = 0; i < numChannels; i++)
        {
            levelMeters.add(std::make_unique<LevelMeter>(p, i));
            addAndMakeVisible(*levelMeters[i]);
        }
    }

    ~LevelMeterGrid() override {}

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid metersGrid;
        metersGrid.templateColumns = { TrackInfo( Fr( 1 ) ) };
        for (auto item : levelMeters)
        {
            metersGrid.templateRows.add( TrackInfo( Fr( 1 ) ) );
            metersGrid.items.add( juce::GridItem(item) );
        }

        metersGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        metersGrid.performLayout(bounds);
    }
    
private:
    VST_SynthAudioProcessor& audioProcessor;

    juce::OwnedArray<LevelMeter> levelMeters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeterGrid)
};