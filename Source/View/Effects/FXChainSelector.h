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
    FXChainSelector(VST_SynthAudioProcessor& p, juce::Array<int>& selectedItems) : audioProcessor(p), selectedItems(selectedItems)
    {
        for (size_t i = 0; i < FXChain::FX_MAX_SLOTS; i++)
        {
            items.add(std::make_unique<FXChainSelectorItem>(p, i, selectedItems));
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

        for (auto item : items)
        {
            chainSelectorGrid.templateRows.add( TrackInfo( Fr( 1 ) ) );
            chainSelectorGrid.items.add(item);
        }

        chainSelectorGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        chainSelectorGrid.performLayout(bounds);
    }


    void updateSelectors()
    {
        selectedItems.clearQuick();
        for(auto item : items)
        {   //collect selected items
            selectedItems.add(item->getChoice());
        }

        for(auto item : items)
        {
            item->updateChoices();
        }
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    juce::Array<int>& selectedItems;

    juce::OwnedArray<FXChainSelectorItem> items;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXChainSelector)
};