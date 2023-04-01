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
    SynthGainComponent(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        addAndMakeVisible(*gainKnob);
        addAndMakeVisible(*levelMeters);
    }

    ~SynthGainComponent() override {}

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        g.setColour(findColour(juce::GroupComponent::outlineColourId));
        g.drawRect(bounds, 1.f);
    }

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;
        
        juce::Grid gainComponentGrid;
        gainComponentGrid.templateRows = { TrackInfo( Fr( 1 ) ) };
        gainComponentGrid.templateColumns = { TrackInfo( Fr( 2 ) ), TrackInfo( Fr( 5 ) ) };
        gainComponentGrid.items = { juce::GridItem( *gainKnob ), juce::GridItem( *levelMeters ) };

        gainComponentGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        gainComponentGrid.performLayout(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<GainKnob> gainKnob = std::make_unique<GainKnob>(audioProcessor);
    std::unique_ptr<LevelMeterGrid> levelMeters = std::make_unique<LevelMeterGrid>(audioProcessor);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthGainComponent)
};