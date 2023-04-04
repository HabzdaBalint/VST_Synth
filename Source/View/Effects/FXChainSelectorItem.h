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

constexpr int IDX_OFFSET = 1;

class FXChainSelectorItem : public juce::Component
{
public:
    FXChainSelectorItem(VST_SynthAudioProcessor& p, int idx) : audioProcessor(p)
    {
        label = std::make_unique<juce::Label>( "slot" + juce::String(idx), "FX Slot #" + juce::String(idx + 1) );
        addAndMakeVisible(*label);
        selector = (std::make_unique<juce::ComboBox>());
        selector->addItemList(FXChain::choices, IDX_OFFSET);
        addAndMakeVisible(*selector);
        bypass = (std::make_unique<juce::ToggleButton>( "Bypass" ));
        addAndMakeVisible(*bypass);
        //todo, attachments and also set items disabled that are loaded on another box
        selectorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            p.apvts,
            FXChain::FXProcessorChain::getFXChoiceParameterName(idx),
            *selector);

        bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            p.apvts,
            FXChain::FXProcessorChain::getFXBypassParameterName(idx),
            *bypass);
    }

    ~FXChainSelectorItem() override {}

    void paint(juce::Graphics& g) override {}

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

    void updateChoices(juce::Array<int> loadedIndexes)
    { //used to invalidate choices in the selector that are already loded in.
        auto choice = selector->getSelectedItemIndex();
        loadedIndexes.removeAllInstancesOf(choice);
        loadedIndexes.removeAllInstancesOf(0);

        for(size_t i = IDX_OFFSET; i < selector->getNumItems() + IDX_OFFSET; i++)
        {
            selector->setItemEnabled(i, true);
        }

        for(auto idx : loadedIndexes)
        {
            selector->setItemEnabled(idx + IDX_OFFSET, false);
        }
    }

    const int getChoice()
    {
        return selector->getSelectedItemIndex();
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<juce::Label> label;

    std::unique_ptr<juce::ComboBox> selector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> selectorAttachment;

    std::unique_ptr<juce::ToggleButton> bypass;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXChainSelectorItem)
};