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

constexpr int HEIGHT_WAVEFORM_EDITOR = 200;
constexpr int PADDING = 4;
constexpr int HEIGHT_PARTIAL_GAIN = 120;
constexpr int HEIGHT_PARTIAL_PHASE = 45;
constexpr int WIDTH_PARTIAL_SLIDERS = 45;

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

    juce::Atomic<bool> parameterChanged{ true };

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

    juce::Slider* gainSlider;
    juce::Slider* phaseSlider;

    juce::AudioProcessorValueTreeState::SliderAttachment* gainSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment* phaseSliderAttachment;

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

    WaveformViewer waveformViewer {audioProcessor};
    WaveformEditor waveformEditor {audioProcessor};
    juce::Viewport waveformEditorViewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorEditor)
};

