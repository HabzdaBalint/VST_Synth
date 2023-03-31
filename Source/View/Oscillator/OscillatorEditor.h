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
        auto bounds = waveformViewer->getBounds();
        g.setColour(findColour(juce::GroupComponent::outlineColourId));
        g.drawRect(bounds, 1.f);

        bounds = waveformEditorViewport->getBounds();
        g.drawRect(bounds, 1.f);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().removeFromTop(getLocalBounds().getHeight() * HEIGHT_WAVEFORM_VIEWER_RELATIVE);
        bounds.reduce(PADDING_PX, PADDING_PX);
        waveformViewer->setBounds(bounds);

        bounds = getLocalBounds().removeFromBottom(getLocalBounds().getHeight() * HEIGHT_WAVEFORM_EDITOR_RELATIVE);
        bounds.reduce(PADDING_PX, PADDING_PX);
        waveformEditorViewport->setBounds(bounds);

        bounds = waveformEditorViewport->getViewedComponent()->getBounds();
        bounds.setHeight(waveformEditorViewport->getLocalBounds().getHeight() - 8);
        waveformEditorViewport->getViewedComponent()->setBounds(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<WaveformViewer> waveformViewer = std::make_unique<WaveformViewer>(audioProcessor);
    std::unique_ptr<juce::Viewport> waveformEditorViewport = std::make_unique<juce::Viewport>();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorEditor)
};