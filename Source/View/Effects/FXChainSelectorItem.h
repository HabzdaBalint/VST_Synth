/*
==============================================================================

    FXChainSelectorItem.h
    Created: 4 Apr 2023 3:39:42pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

class FXChainSelectorItem : public juce::Component
{
public:
    FXChainSelectorItem(VST_SynthAudioProcessor& p, int idx) : audioProcessor(p)
    {
        label = std::make_unique<juce::Label>( "slot" + juce::String(idx), "FX Slot #" + juce::String(idx + 1) );
        addAndMakeVisible(*label);
        selector = (std::make_unique<juce::ComboBox>());
        addAndMakeVisible(*selector);
        bypass = (std::make_unique<juce::ToggleButton>( "Bypass" ));
        addAndMakeVisible(*bypass);
        //todo, attachments and also set items disabled that are loaded on another box
        selectorAttachment = std::make_unique<juce::ComboBoxParameterAttachment>();

    }

    ~FXChainSelectorItem() override {}

    void paint(juce::Graphics& g) override 
    {
        g.setColour(findColour(juce::GroupComponent::outlineColourId));
        auto bounds = bypass->getBounds();
        g.drawRect(bounds, 1.f);
    }

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid chainSelectorItemGrid;
        chainSelectorItemGrid.templateColumns = { TrackInfo( Fr( 5 ) ), TrackInfo( Fr( 2 ) ) };
        chainSelectorItemGrid.templateRows = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 3 ) ) };
        chainSelectorItemGrid.items = { juce::GridItem( *label ), nullptr,
                                        juce::GridItem( *selector ), juce::GridItem( *bypass ) };

        chainSelectorItemGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        chainSelectorItemGrid.performLayout(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<juce::Label> label;

    std::unique_ptr<juce::ComboBox> selector;
    std::unique_ptr<juce::ComboBoxParameterAttachment> selectorAttachment;

    std::unique_ptr<juce::ToggleButton> bypass;
    std::unique_ptr<juce::ButtonParameterAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXChainSelectorItem)
};