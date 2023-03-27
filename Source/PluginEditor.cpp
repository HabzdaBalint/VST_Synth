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
    setLookAndFeel(&lnf);

    setSize (700, 500);
    setResizable(false, false);

    addAndMakeVisible(keyboardComponent);
    keyboardComponent.setLookAndFeel(&lnf);

    addAndMakeVisible(tabbedComponent);
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

VST_SynthTabbedComponent::VST_SynthTabbedComponent(VST_SynthAudioProcessor& p, juce::LookAndFeel_V4* lnf)
    : audioProcessor (p),
      TabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    setLookAndFeel(lnf);

    auto color = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);

    synthComponent = audioProcessor.additiveSynth->createEditor();
    fxComponent = audioProcessor.fxChain->createEditor();

    addTab("Synth", color, synthComponent, false, 0);
    //getTabContentComponent(0)->setLookAndFeel(lnf);
    addTab("FX", color, fxComponent, false, 1);
    //getTabContentComponent(1)->setLookAndFeel(lnf);
}

VST_SynthTabbedComponent::~VST_SynthTabbedComponent()
{
    delete(synthComponent);
    delete(fxComponent);
}
