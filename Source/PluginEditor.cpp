/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VST_SynthAudioProcessorEditor::VST_SynthAudioProcessorEditor (VST_SynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(keyboardComponent);

    addAndMakeVisible(tabbedComponent);

    setSize (700, 500);
    setResizable(false, false);

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

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void VST_SynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    keyboardComponent.setBounds(0, getBounds().getBottom() - 100, 700, 100);

    tabbedComponent.setBounds(0, 0, 700, 400);
}

//===================================================================================================================

VST_SynthTabbedComponent::VST_SynthTabbedComponent(VST_SynthAudioProcessor& p)
    : audioProcessor (p),
      TabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    auto color = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);

    addTab("Synth", color, audioProcessor.additiveSynth->createEditor(), false, 0);
    addTab("FX", color, audioProcessor.fxChain->createEditor(), false, 1);
}

VST_SynthTabbedComponent::~VST_SynthTabbedComponent() {}
