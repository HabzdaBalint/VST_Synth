/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "View/EditorParameters.h"
#include "View/Oscillator/OscillatorEditor.h"
#include "View/Synthesizer/SynthEditor.h"
#include "View/Effects/EffectsEditor.h"

#include "View/VST_SynthLookAndFeel.h"

class VST_SynthTabbedComponent : public juce::TabbedComponent
{
public:
    VST_SynthTabbedComponent(VST_SynthAudioProcessor&);
    ~VST_SynthTabbedComponent() override;
private:
    VST_SynthAudioProcessor& audioProcessor;

    void lookAndFeelChanged();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VST_SynthTabbedComponent)
};

//============================================================================================================

using MIDIKeyboard = juce::MidiKeyboardComponent;

class VST_SynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    VST_SynthAudioProcessorEditor (VST_SynthAudioProcessor&);
    ~VST_SynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    VST_SynthAudioProcessor& audioProcessor;

    VST_SynthLookAndFeel lnf = VST_SynthLookAndFeel();

    std::unique_ptr<VST_SynthTabbedComponent> tabbedComponent = std::make_unique<VST_SynthTabbedComponent>(audioProcessor);

    std::unique_ptr<MIDIKeyboard> keyboardComponent = std::make_unique<MIDIKeyboard>(audioProcessor.keyboardState, MIDIKeyboard::horizontalKeyboard);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VST_SynthAudioProcessorEditor)
};
