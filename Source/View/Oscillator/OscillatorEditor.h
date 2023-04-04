/*
==============================================================================

    OscillatorEditor.h
    Created: 18 Mar 2023 8:34:04pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"
#include "WaveformViewer.h"
#include "WaveformEditor.h"

class OscillatorEditor : public juce::Component
{
public:
    OscillatorEditor(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        addAndMakeVisible(*waveformViewer);
        waveformEditorViewport->setViewedComponent(new WaveformEditor(p), true);
        addAndMakeVisible(*waveformEditorViewport);
    }

    ~OscillatorEditor() override {}

    void paint(juce::Graphics& g) override
    {
        g.setColour(findColour(juce::GroupComponent::outlineColourId));
        auto bounds = waveformEditorViewport->getBounds();
        g.drawRect(bounds, 1.f);
    }

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid oscillatorEditorGrid;
        oscillatorEditorGrid.templateRows = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
        oscillatorEditorGrid.templateColumns = { TrackInfo( Fr( 1 ) ) };
        oscillatorEditorGrid.items = { juce::GridItem( *waveformViewer ), 
                                       juce::GridItem( *waveformEditorViewport ) };

        oscillatorEditorGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        oscillatorEditorGrid.performLayout(bounds);

        bounds = waveformEditorViewport->getViewedComponent()->getBounds();
        bounds.setHeight(waveformEditorViewport->getLocalBounds().getHeight() - 8); //8 is the height of the scroll bar
        waveformEditorViewport->getViewedComponent()->setBounds(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<WaveformViewer> waveformViewer = std::make_unique<WaveformViewer>(audioProcessor);
    std::unique_ptr<juce::Viewport> waveformEditorViewport = std::make_unique<juce::Viewport>();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorEditor)
};