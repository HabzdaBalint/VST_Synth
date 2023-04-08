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

class EffectsEditor : public juce::Component,
                      public juce::AudioProcessorParameter::Listener
{
public:
    EffectsEditor(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        addAndMakeVisible(*chainSelector);
        chainEditorViewport->setViewedComponent(new FXChainEditor(p, selectedEffects), true);
        addAndMakeVisible(*chainEditorViewport);

        for (size_t i = 0; i < EffectsChain::FX_MAX_SLOTS; i++)
        {
            p.apvts.getParameter(EffectsChain::FXProcessorChain::getFXChoiceParameterName(i))->addListener(this);
        }
    }

    ~EffectsEditor() override
    {
        for (size_t i = 0; i < EffectsChain::FX_MAX_SLOTS; i++)
        {
            audioProcessor.apvts.getParameter(EffectsChain::FXProcessorChain::getFXChoiceParameterName(i))->removeListener(this);
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

        juce::Grid effectsEditorGrid;
        effectsEditorGrid.templateRows = { TrackInfo( Fr( 1 ) ) };
        effectsEditorGrid.templateColumns = { TrackInfo( Fr( 3 ) ), TrackInfo( Fr( 8 ) ) };
        effectsEditorGrid.items = { juce::GridItem( *chainSelector ), juce::GridItem( *chainEditorViewport ) };

        effectsEditorGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        effectsEditorGrid.performLayout(bounds);

        bounds = chainEditorViewport->getViewedComponent()->getBounds();
        bounds.setWidth(chainEditorViewport->getLocalBounds().getWidth());
        chainEditorViewport->getViewedComponent()->setBounds(bounds);
    }

    void parameterValueChanged (int parameterIndex, float newValue) override
    {
        chainSelector->updateSelectors();
        auto editor = dynamic_cast<FXChainEditor*>(chainEditorViewport->getViewedComponent());
        editor->updateChainEditor();
    }

    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}

private:
    VST_SynthAudioProcessor& audioProcessor;

    juce::Array<int> selectedEffects;

    std::unique_ptr<FXChainSelector> chainSelector = std::make_unique<FXChainSelector>(audioProcessor, selectedEffects);
    std::unique_ptr<juce::Viewport> chainEditorViewport = std::make_unique<juce::Viewport>();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsEditor)
};