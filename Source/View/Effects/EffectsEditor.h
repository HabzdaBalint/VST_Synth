/*
==============================================================================

    EffectsEditor.h
    Created: 4 Apr 2023 1:40:29pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "FXChainEditor.h"
#include "FXChainSelector.h"

class EffectsEditor : public juce::Component
{
public:
    EffectsEditor(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        addAndMakeVisible(*chainSelector);
        chainEditorViewport->setViewedComponent(new FXChainEditor(p), true);
        addAndMakeVisible(*chainEditorViewport);
    }

    ~EffectsEditor() override {}

    void paint(juce::Graphics& g) override
    {
        g.setColour(findColour(juce::GroupComponent::outlineColourId));
        auto bounds = chainEditorViewport->getBounds();
        g.drawRect(bounds, 1.f);

        bounds = chainSelector->getBounds();
        g.drawRect(bounds, 1.f);
    }

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid effectsEditorGrid;
        effectsEditorGrid.templateRows = { TrackInfo( Fr( 1 ) ) };
        effectsEditorGrid.templateColumns = { TrackInfo( Fr( 3 ) ), TrackInfo( Fr( 5 ) ) };
        effectsEditorGrid.items = { juce::GridItem( *chainSelector ), juce::GridItem( *chainEditorViewport ) };

        effectsEditorGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        effectsEditorGrid.performLayout(bounds);

        bounds = chainEditorViewport->getViewedComponent()->getBounds();
        bounds.setWidth(chainEditorViewport->getLocalBounds().getWidth() - 8); //8 is the width of the scroll bar
        chainEditorViewport->getViewedComponent()->setBounds(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<FXChainSelector> chainSelector = std::make_unique<FXChainSelector>(audioProcessor);
    std::unique_ptr<juce::Viewport> chainEditorViewport = std::make_unique<juce::Viewport>();

    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsEditor)
};