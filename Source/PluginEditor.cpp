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

    setSize (WIDTH_MAIN_WINDOW, HEIGHT_MAIN_WINDOW);
    setResizable(false, false);

    addAndMakeVisible(tabbedComponent);
    addAndMakeVisible(keyboardComponent);

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

    //g.setColour (juce::Colours::white);
    //g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void VST_SynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    keyboardComponent.setBounds(0, getBounds().getBottom() - HEIGHT_KEYBOARD, WIDTH_MAIN_WINDOW, HEIGHT_KEYBOARD);

    tabbedComponent.setBounds(0, 0, WIDTH_MAIN_WINDOW, HEIGHT_MAIN_WINDOW-HEIGHT_KEYBOARD);
}

//===================================================================================================================

VST_SynthTabbedComponent::VST_SynthTabbedComponent(VST_SynthAudioProcessor& p)
    : audioProcessor (p),
      TabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    auto color = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);

    addTab("Oscillator", color, &oscillatorComponent, true);
    addTab("Synth", color, nullptr /*&synthComponent*/, true);  //todo
    addTab("FX", color, nullptr /*&fxChainComponent*/, true);
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

