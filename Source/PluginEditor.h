/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


class VST_SynthTabbedComponent : public juce::TabbedComponent
{
public:
    VST_SynthTabbedComponent(VST_SynthAudioProcessor&);
    ~VST_SynthTabbedComponent() override;
private:
    VST_SynthAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VST_SynthTabbedComponent)
};

//==============================================================================
/**
*/
class VST_SynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    VST_SynthAudioProcessorEditor (VST_SynthAudioProcessor&);
    ~VST_SynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    VST_SynthAudioProcessor& audioProcessor;

    VST_SynthTabbedComponent tabbedComponent { audioProcessor };

    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent { keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard };
    juce::LookAndFeel_V4 lnf = juce::LookAndFeel_V4(juce::LookAndFeel_V4::getMidnightColourScheme());

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST_SynthAudioProcessorEditor)
};
