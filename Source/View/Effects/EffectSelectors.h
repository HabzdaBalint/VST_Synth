/*
==============================================================================

    EffectSelectors.h
    Created: 4 Apr 2023 1:38:03pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "EffectSelector.h"

class EffectSelectors : public juce::Component,
                        public juce::AudioProcessorValueTreeState::Listener,
                        public juce::ComboBox::Listener
{
public:
    EffectSelectors(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        for (size_t i = 0; i < Effects::EffectsChain::FX_MAX_SLOTS; i++)
        {
            selectors.add(std::make_unique<EffectSelector>(p, i, selectedIndexes));
            addAndMakeVisible(*selectors[i]);
            selectors[i]->getSelector().addListener(this);
        }

        for (size_t i = 0; i < Effects::EffectsChain::FX_MAX_SLOTS; i++)
        {
            audioProcessor.apvts.addParameterListener(Effects::EffectsChain::getFXChoiceParameterID(i), this);
        }
    }

    ~EffectSelectors() override
    {
        for(size_t i = 0; i < Effects::EffectsChain::FX_MAX_SLOTS; i++)
        {
            audioProcessor.apvts.removeParameterListener(Effects::EffectsChain::getFXChoiceParameterID(i), this);
        }

        for(auto selector : selectors)
        {
            selector->getSelector().removeListener(this);
        }
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(findColour(juce::GroupComponent::outlineColourId));

        for(auto child : getChildren())
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

        for(auto selector : selectors)
        {
            chainSelectorGrid.templateRows.add( TrackInfo( Fr( 1 ) ) );
            chainSelectorGrid.items.add(selector);
        }

        chainSelectorGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        chainSelectorGrid.performLayout(bounds);
    }

    void updateSelectors()
    {
        selectedIndexes.clearQuick();
        for(auto selector : selectors)
        {   //collect selected selector indexes
            selectedIndexes.add(selector->getSelector().getSelectedItemIndex());
        }

        for(auto selector : selectors)
        {
            selector->updateChoices();
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

    juce::Array<int> selectedIndexes;

    juce::OwnedArray<EffectSelector> selectors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectSelectors)
};