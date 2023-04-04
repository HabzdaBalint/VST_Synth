/*
==============================================================================

    FXBypassButton.h
    Created: 4 Apr 2023 3:01:30pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

class FXBypassButton : public juce::ToggleButton
{
public:
    FXBypassButton(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        for (size_t i = 0; i < FXChain::FX_MAX_SLOTS; i++)
        {
            selectors.add(std::make_unique<juce::ComboBox>());
            addAndMakeVisible(*selectors[i]);
            bypasses.add(std::make_unique<juce::ToggleButton>());
            addAndMakeVisible(*bypasses[i]);
        }
    }

    ~FXBypassButton() override {}

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid chainSelectorGrid;
        chainSelectorGrid.templateColumns = { TrackInfo( Fr( 3 ) ), TrackInfo( Fr( 1 ) ) };

        for (size_t i = 0; i < FXChain::FX_MAX_SLOTS; i++)
        {
            chainSelectorGrid.templateRows.add( TrackInfo( Fr( 1 ) ) );
            chainSelectorGrid.items.add(selectors[i]);
            chainSelectorGrid.items.add(bypasses[i]);
        }

        chainSelectorGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        chainSelectorGrid.performLayout(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    juce::OwnedArray<juce::ComboBox> selectors;
    juce::OwnedArray<juce::ToggleButton> bypasses;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXChainSelector)
};