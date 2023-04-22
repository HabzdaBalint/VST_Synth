/*
==============================================================================

    EffectUnitSelectors.h
    Created: 4 Apr 2023 1:38:03pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "EffectUnitSelector.h"

class EffectUnitSelectors : public juce::Component,
                            public juce::AudioProcessorValueTreeState::Listener,
                            public juce::ComboBox::Listener
{
public:
    EffectUnitSelectors(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        for (size_t i = 0; i < Effects::EffectsChain::FX_MAX_SLOTS; i++)
        {
            items.add(std::make_unique<EffectUnitSelector>(p, i, selectedItems));
            addAndMakeVisible(*items[i]);
            items[i]->getSelector().addListener(this);
        }

        for (size_t i = 0; i < Effects::EffectsChain::FX_MAX_SLOTS; i++)
        {
            audioProcessor.apvts.addParameterListener(Effects::EffectsChain::getFXChoiceParameterName(i), this);
        }
    }

    ~EffectUnitSelectors() override
    {
        for (size_t i = 0; i < Effects::EffectsChain::FX_MAX_SLOTS; i++)
        {
            audioProcessor.apvts.removeParameterListener(Effects::EffectsChain::getFXChoiceParameterName(i), this);
        }

        for (auto item : items)
        {
            item->getSelector().removeListener(this);
        }
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(findColour(juce::GroupComponent::outlineColourId));

        for ( auto child : getChildren() )
        {
            auto bounds = child->getBounds();
            g.drawRoundedRectangle(bounds.toFloat(), 4.f, OUTLINE_WIDTH);
        }
    }

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
        {   //collect selected item indexes
            selectedItems.add(item->getChoice());
        }

        for(auto item : items)
        {
            item->updateChoices();
        }
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        updateSelectors();
    }

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override
    {
        updateSelectors();
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    juce::Array<int> selectedItems;

    juce::OwnedArray<EffectUnitSelector> items;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectUnitSelectors)
};