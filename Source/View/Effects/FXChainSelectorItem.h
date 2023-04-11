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
    FXChainSelectorItem(VST_SynthAudioProcessor& p, int idx, juce::Array<int>& selectedIndexes) : audioProcessor(p), selectedIndexes(selectedIndexes)
    {
        selector = std::make_unique<juce::ComboBox>();
        selectorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            p.apvts,
            EffectsChain::FXProcessorChain::getFXChoiceParameterName(idx),
            *selector);
        selector->addItemList(EffectsChain::choices, IDX_OFFSET);
        selector->setSelectedItemIndex(0);
        addAndMakeVisible(*selector);
        
        bypass = std::make_unique<juce::ToggleButton>( "Bypass" );
        bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            p.apvts,
            EffectsChain::FXProcessorChain::getFXBypassParameterName(idx),
            *bypass);
        addAndMakeVisible(*bypass);
        
        label = std::make_unique<juce::Label>( "slot" + juce::String(idx), "FX Slot #" + juce::String(idx + 1) );
        addAndMakeVisible(*label);
    }

    ~FXChainSelectorItem() override {}

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid chainSelectorItemGrid;
        chainSelectorItemGrid.templateColumns = { TrackInfo( Fr( 3 ) ), TrackInfo( Fr( 2 ) ) };
        chainSelectorItemGrid.templateRows = { TrackInfo( Fr( 1 ) ), TrackInfo( Px( 10 ) ), TrackInfo( Px( 25 ) ), TrackInfo( Fr( 1 ) ) };
        chainSelectorItemGrid.items = { juce::GridItem( *label ).withColumn( { 1, 3 } ).withRow( { 2 } ),
                                        juce::GridItem( *selector ).withColumn( { 1 } ).withRow( { 3 } ),
                                        juce::GridItem( *bypass ).withColumn( { 2 } ).withRow( { 3 } ) };

        chainSelectorItemGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        chainSelectorItemGrid.performLayout(bounds);
    }

    void updateChoices()
    {   //used to disable effect choices in the selector that are already loded in.
        auto choice = getChoice();

        for(size_t i = 0; i < selector->getNumItems(); i++)
        {
            selector->setItemEnabled(i + IDX_OFFSET, true); //setItemEnabled works with item IDs and not indexes, so the offset is needed to address them
        }

        for(auto idx : selectedIndexes)
        {
            if(idx != choice && idx != 0)
                selector->setItemEnabled(idx + IDX_OFFSET, false);
        }
    }

    const int getChoice()
    {
        return selector->getSelectedItemIndex();
    }

    juce::ComboBox& getSelector()
    {
        return *selector;
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    const juce::Array<int>& selectedIndexes;

    std::unique_ptr<juce::Label> label;

    std::unique_ptr<juce::ComboBox> selector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> selectorAttachment;

    std::unique_ptr<juce::ToggleButton> bypass;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXChainSelectorItem)
};