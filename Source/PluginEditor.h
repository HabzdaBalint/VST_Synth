/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "View/OscillatorEditor.h"
#include "View/FXProcessorChainEditor.h"

constexpr int WIDTH_MAIN_WINDOW = 700;
constexpr int HEIGHT_MAIN_WINDOW = 550;
constexpr int HEIGHT_KEYBOARD = 80;

class VST_SynthTabbedComponent : public juce::TabbedComponent
{
public:
    VST_SynthTabbedComponent(VST_SynthAudioProcessor&);
    ~VST_SynthTabbedComponent() override;
private:
    VST_SynthAudioProcessor& audioProcessor;

    OscillatorEditor oscillatorComponent = {audioProcessor}; 
    //SynthEditor synthComponent {audioProcessor};
    //FXChainEditor fxChainComponent {audioProcessor};

    void lookAndFeelChanged() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VST_SynthTabbedComponent)
};


class VST_SynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    VST_SynthAudioProcessorEditor (VST_SynthAudioProcessor&);
    ~VST_SynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    VST_SynthAudioProcessor& audioProcessor;

    juce::LookAndFeel_V4 lnf = juce::LookAndFeel_V4(juce::LookAndFeel_V4::getMidnightColourScheme());

    VST_SynthTabbedComponent tabbedComponent {audioProcessor};

    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent {keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VST_SynthAudioProcessorEditor)
};
