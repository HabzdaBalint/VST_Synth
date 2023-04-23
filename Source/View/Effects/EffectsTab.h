/*
==============================================================================

    EffectsTab.h
    Created: 4 Apr 2023 1:40:29pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "EffectEditors.h"
#include "EffectSelectors.h"

class EffectsTab : public juce::Component
{
public:
    EffectsTab(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        addAndMakeVisible(*chainSelectors);
        chainEditorsViewport->setViewedComponent(new EffectEditors(p), true);
        addAndMakeVisible(*chainEditorsViewport);
    }

    ~EffectsTab() override {}

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

        juce::Grid effectsEditorGrid;
        effectsEditorGrid.templateRows = { TrackInfo( Fr( 1 ) ) };
        effectsEditorGrid.templateColumns = { TrackInfo( Fr( 3 ) ), TrackInfo( Fr( 8 ) ) };
        effectsEditorGrid.items = { juce::GridItem( *chainSelectors ), juce::GridItem( *chainEditorsViewport ) };

        effectsEditorGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        effectsEditorGrid.performLayout(bounds);

        bounds = chainEditorsViewport->getViewedComponent()->getBounds();
        bounds.setWidth(chainEditorsViewport->getLocalBounds().getWidth());
        chainEditorsViewport->getViewedComponent()->setBounds(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<EffectSelectors> chainSelectors = std::make_unique<EffectSelectors>(audioProcessor);
    std::unique_ptr<juce::Viewport> chainEditorsViewport = std::make_unique<juce::Viewport>();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsTab)
};