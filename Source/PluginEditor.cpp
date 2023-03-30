/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VST_SynthAudioProcessorEditor::VST_SynthAudioProcessorEditor (VST_SynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setSize(WIDTH_MAIN_WINDOW_PX, HEIGHT_MAIN_WINDOW_PX);
    setResizable(true, false);
    setResizeLimits(WIDTH_MAIN_WINDOW_PX, HEIGHT_MAIN_WINDOW_PX, 1.3 * WIDTH_MAIN_WINDOW_PX, 1.3 * HEIGHT_MAIN_WINDOW_PX);
    addAndMakeVisible(*tabbedComponent);
    addAndMakeVisible(*keyboardComponent);

    setLookAndFeel(&lnf);
}

VST_SynthAudioProcessorEditor::~VST_SynthAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void VST_SynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void VST_SynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds().removeFromBottom(HEIGHT_KEYBOARD_PX);;
    keyboardComponent->setBounds(bounds);

    bounds = getLocalBounds().removeFromTop(getLocalBounds().getHeight() - HEIGHT_KEYBOARD_PX);
    tabbedComponent->setBounds(bounds);
}

//===================================================================================================================

VST_SynthTabbedComponent::VST_SynthTabbedComponent(VST_SynthAudioProcessor& p)
    : audioProcessor (p),
      TabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    auto color = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);

    addTab("Oscillator", color, new OscillatorEditor(p), true);
    addTab("Synthesizer", color, new SynthEditor(p), true);  
    addTab("Effects", color, nullptr /*&new FXProcessorChainEditor(p)*/, true); //todo
}

VST_SynthTabbedComponent::~VST_SynthTabbedComponent() {}

void VST_SynthTabbedComponent::lookAndFeelChanged()
{
    auto color = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
    for (size_t i = 0; i < getNumTabs(); i++)
    {
        setTabBackgroundColour(i, color);
    }
}

