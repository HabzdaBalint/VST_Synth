/*
==============================================================================

    OscillatorEditor.h
    Created: 18 Mar 2023 8:34:04pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "EditorParameters.h"


class WaveformViewer : public juce::Component,
                       public juce::Timer,
                       public juce::AudioProcessorParameter::Listener
{
public:
    WaveformViewer(VST_SynthAudioProcessor&);
    ~WaveformViewer() override;

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}

    void timerCallback() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    VST_SynthAudioProcessor& audioProcessor;

    juce::Path waveformPath;

    juce::Atomic<bool> parameterChanged { false };

    void redrawPath();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformViewer)
};

//=================================================================

class PartialSliders : public juce::Component
{
public:
    PartialSliders(VST_SynthAudioProcessor&, int);
    ~PartialSliders() override;

    void paint(juce::Graphics&) override;
    void resized() override;
private:
    VST_SynthAudioProcessor& audioProcessor;

    int partialIndex;

    std::unique_ptr<juce::Slider> gainSlider;
    std::unique_ptr<juce::Slider> phaseSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> phaseSliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PartialSliders)
};

//=================================================================

class WaveformEditor : public juce::Component
{
public:
    WaveformEditor(VST_SynthAudioProcessor&);
    ~WaveformEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
private:
    VST_SynthAudioProcessor& audioProcessor;

    juce::OwnedArray<PartialSliders> partialSliders;

    void createGainSlider(int idx);
    void createPhaseSlider(int idx);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformEditor)
};

//=================================================================

class OscillatorEditor : public juce::Component
{
public:
    OscillatorEditor(VST_SynthAudioProcessor&);
    ~OscillatorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<WaveformViewer> waveformViewer = std::make_unique<WaveformViewer>(audioProcessor);
    std::unique_ptr<juce::Viewport> waveformEditorViewport = std::make_unique<juce::Viewport>();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorEditor)
};

