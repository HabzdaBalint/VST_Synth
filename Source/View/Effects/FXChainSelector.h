/*
==============================================================================

    FXChainSelector.h
    Created: 4 Apr 2023 1:38:03pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "FXChainSelectorItem.h"

class FXChainSelector : public juce::Component
{
public:
    FXChainSelector(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        for (size_t i = 0; i < FXChain::FX_MAX_SLOTS; i++)
        {
            items.add(std::make_unique<FXChainSelectorItem>(p, i));
            addAndMakeVisible(*items[i]);
        }
    }

    ~FXChainSelector() override {}

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid chainSelectorGrid;
        chainSelectorGrid.templateColumns = { TrackInfo( Fr( 1 ) ) };

        for (size_t i = 0; i < FXChain::FX_MAX_SLOTS; i++)
        {
            chainSelectorGrid.templateRows.add( TrackInfo( Fr( 1 ) ) );
            chainSelectorGrid.items.add(items[i]);
        }

        chainSelectorGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        chainSelectorGrid.performLayout(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    juce::OwnedArray<FXChainSelectorItem> items;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXChainSelector)
};