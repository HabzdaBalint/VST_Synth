/*
==============================================================================

    EffectSelector.h
    Created: 4 Apr 2023 3:39:42pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

class EffectSelector : public juce::Component
{
public:
    EffectSelector(VST_SynthAudioProcessor& p, int idx, juce::Array<int>& selectedIndexes) : audioProcessor(p), selectedIndexes(selectedIndexes)
    {
        selector = std::make_unique<juce::ComboBox>();
        selectorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            p.apvts,
            Effects::EffectsChain::getFXChoiceParameterID(idx),
            *selector);
        selector->addItemList(Effects::EffectsChain::choices, IDX_OFFSET);
        selector->setSelectedItemIndex(p.apvts.getRawParameterValue(Effects::EffectsChain::getFXChoiceParameterID(idx))->load());
        addAndMakeVisible(*selector);
        
        bypass = std::make_unique<juce::ToggleButton>( "Bypass" );
        bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            p.apvts,
            Effects::EffectsChain::getFXBypassParameterID(idx),
            *bypass);
        addAndMakeVisible(*bypass);
        
        label = std::make_unique<juce::Label>( "slot" + juce::String(idx), "FX Slot #" + juce::String(idx + 1) );
        addAndMakeVisible(*label);
    }

    ~EffectSelector() override {}

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
        auto choice = selector->getSelectedItemIndex();

        for(size_t i = 0; i < selector->getNumItems(); i++)
        {
            selector->setItemEnabled(selector->getItemId(i), true);
        }

        for(auto idx : selectedIndexes)
        {
            if(idx != choice && idx != 0)
                selector->setItemEnabled(selector->getItemId(idx), false);
        }
    }

    juce::ComboBox& getSelector() const
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectSelector)
};