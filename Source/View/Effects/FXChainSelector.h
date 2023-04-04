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

class FXChainSelector : public juce::Component,
                        public juce::AudioProcessorParameter::Listener
{
public:
    FXChainSelector(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        for (size_t i = 0; i < FXChain::FX_MAX_SLOTS; i++)
        {
            items.add(std::make_unique<FXChainSelectorItem>(p, i));
            addAndMakeVisible(*items[i]);

            p.apvts.getParameter(FXChain::FXProcessorChain::getFXChoiceParameterName(i))->addListener(this);
        }
    }

    ~FXChainSelector() override
    {
        for (size_t i = 0; i < FXChain::FX_MAX_SLOTS; i++)
        {
            audioProcessor.apvts.getParameter(FXChain::FXProcessorChain::getFXChoiceParameterName(i))->removeListener(this);
        }
    }

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

    void parameterValueChanged (int parameterIndex, float newValue) override
    {
        updateSelectors();
    }

    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}

    void updateSelectors()
    {
        juce::Array<int> selectedItems;
        for(auto item : items)
        {   //collect selected items
            selectedItems.add(item->getChoice());
        }

        for(auto item : items)
        {
            item->updateChoices(selectedItems);
        }
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    juce::OwnedArray<FXChainSelectorItem> items;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXChainSelector)
};