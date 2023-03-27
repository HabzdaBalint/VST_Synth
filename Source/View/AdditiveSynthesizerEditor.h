/*
==============================================================================

    AdditiveSynthesizerEditor.h
    Created: 18 Mar 2023 8:34:04pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../DataModel/AdditiveSynthesizer.h"

class AdditiveSynthesizerEditor : public juce::AudioProcessorEditor,
                                  public juce::Timer,
                                  public juce::AudioProcessorParameter::Listener
{
public:
    AdditiveSynthesizerEditor(AdditiveSynthesizer&, juce::AudioProcessorValueTreeState*);

    ~AdditiveSynthesizerEditor() override;

    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override {}

    void timerCallback() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AdditiveSynthesizer& audioProcessor;
    juce::AudioProcessorValueTreeState* apvts;

    juce::Path waveformPath;

    juce::Atomic<bool> parameterChanged {true};

    void redrawPath();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthesizerEditor)
};