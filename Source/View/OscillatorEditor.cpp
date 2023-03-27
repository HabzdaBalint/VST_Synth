/*
==============================================================================

    OscillatorEditor.cpp
    Created: 25 Mar 2023 9:08:06pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "OscillatorEditor.h"

OscillatorEditor::OscillatorEditor(VST_SynthAudioProcessor& p) : audioProcessor(p)
{
    addAndMakeVisible(waveformViewer);
    waveformEditorViewport.setViewedComponent(&waveformEditor, false);
    addAndMakeVisible(waveformEditorViewport);
}

OscillatorEditor::~OscillatorEditor() {}

void OscillatorEditor::paint (juce::Graphics& g)
{
    auto bounds = waveformViewer.getBounds();
    g.setColour(getLookAndFeel().findColour(juce::GroupComponent::outlineColourId));
    g.drawRect(bounds, 1.f);

    bounds = waveformEditorViewport.getBounds();
    g.drawRect(bounds, 1.f);
}

void OscillatorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(PADDING, PADDING);
    bounds.removeFromBottom(HEIGHT_WAVEFORM_EDITOR);
    waveformViewer.setBounds(bounds);

    // waveformViewer.setBoundsInset();

    // juce::BorderSize<int> border;
    // border.setBottom(PADDING);
    // border.setLeft(PADDING);
    // border.setRight(PADDING);
    // border.setTop(PADDING);

    bounds = getLocalBounds();
    bounds.reduce(PADDING, PADDING);
    bounds.removeFromTop(bounds.getHeight() - HEIGHT_WAVEFORM_EDITOR);
    waveformEditorViewport.setBounds(bounds);

    bounds = waveformEditorViewport.getViewedComponent()->getBounds();
    bounds.setHeight(waveformEditorViewport.getLocalBounds().getHeight() - 8);
    waveformEditorViewport.getViewedComponent()->setBounds(bounds);
}

//=========================================================================================================

WaveformViewer::WaveformViewer(VST_SynthAudioProcessor& p) : audioProcessor(p)
{
    for (size_t i = 0; i < HARMONIC_N; i++)
    {
        audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialGainParameterName(i))->addListener(this);
        audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialPhaseParameterName(i))->addListener(this);
    }

    startTimerHz(60);

    redrawPath();
}

WaveformViewer::~WaveformViewer()
{
    for (size_t i = 0; i < HARMONIC_N; i++)
    {
        audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialGainParameterName(i))->removeListener(this);
        audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialPhaseParameterName(i))->removeListener(this);
    }
}

void WaveformViewer::paint (juce::Graphics& g)
{
    g.setColour(findColour(juce::Slider::textBoxTextColourId));
    g.strokePath(waveformPath, juce::PathStrokeType(1.5));
}

void WaveformViewer::resized() {}

void WaveformViewer::parameterValueChanged (int parameterIndex, float newValue)
{
    parameterChanged.set(true);
}

void WaveformViewer::timerCallback()
{
    if( parameterChanged.compareAndSetBool(false, true) )
    {
        redrawPath();
        repaint();
    }
}

void WaveformViewer::redrawPath()
{
    waveformPath.clear();

    std::vector<float> amplitudes;
    amplitudes.resize(LOOKUP_POINTS);

    for (size_t i = 0; i < LOOKUP_POINTS; i++)
    {
        amplitudes[i] = audioProcessor.additiveSynth->WaveTableFormula((juce::MathConstants<float>::twoPi * (float)i) / (float)LOOKUP_POINTS, HARMONIC_N);
    }

    waveformPath.preallocateSpace(3*LOOKUP_POINTS);
    waveformPath.startNewSubPath(getX(), -1 * amplitudes.front());
    for (size_t i = 1; i < amplitudes.size(); i++)
    {
        waveformPath.lineTo(getX() + i, -1 * amplitudes[i]);
    }

    int zeroCount = 0;
    for (size_t i = 0; i < amplitudes.size(); i++)
    {
        if (amplitudes[i] == 0)
            zeroCount++;
    }
    
    if (zeroCount != amplitudes.size())
    {
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING, PADDING);

        waveformPath.scaleToFit(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), false);
    }
    else
    {
        //todo: draw a straight line in the middle
    }
}

//=========================================================================================================

WaveformEditor::WaveformEditor(VST_SynthAudioProcessor& p) : audioProcessor(p)
{
    for (size_t i = 0; i < HARMONIC_N; i++)
    {
        partialSliders.add(new PartialSliders(audioProcessor, i));
        addAndMakeVisible(partialSliders[i]);
    }
    
    auto bounds = getLocalBounds();
    bounds.setWidth(HARMONIC_N * WIDTH_PARTIAL_SLIDERS);
    setBounds(bounds);
}

WaveformEditor::~WaveformEditor()
{
}

void WaveformEditor::paint(juce::Graphics& g)
{
    
}

void WaveformEditor::resized()
{
    auto bounds1 = getLocalBounds();

    for (size_t i = 0; i < HARMONIC_N; i++)
    {
        auto bounds = getLocalBounds();
        bounds.setWidth(WIDTH_PARTIAL_SLIDERS);
        bounds.setX(i * WIDTH_PARTIAL_SLIDERS);
        partialSliders[i]->setBounds(bounds);
    }
}

//=========================================================================================================

PartialSliders::PartialSliders(VST_SynthAudioProcessor& p, int idx) : audioProcessor(p), partialIndex(idx)
{
    gainSlider = new juce::Slider(juce::Slider::SliderStyle::LinearBarVertical,
                                  juce::Slider::TextEntryBoxPosition::TextBoxBelow);
    gainSliderAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.apvts,
                                                        audioProcessor.additiveSynth->getPartialGainParameterName(partialIndex),
                                                        *gainSlider);
    gainSlider->setScrollWheelEnabled(false);
    gainSlider->setTextValueSuffix("%");
    gainSlider->setTextBoxIsEditable(false);
    addAndMakeVisible(gainSlider);

    phaseSlider = new juce::Slider(juce::Slider::SliderStyle::LinearBar,
                                   juce::Slider::TextEntryBoxPosition::TextBoxBelow);
    phaseSliderAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.apvts,
                                                        audioProcessor.additiveSynth->getPartialPhaseParameterName(partialIndex),
                                                        *phaseSlider);    
    phaseSlider->setScrollWheelEnabled(false);
    phaseSlider->setTextValueSuffix("%");
    phaseSlider->setTextBoxIsEditable(false);
    addAndMakeVisible(phaseSlider);
}

PartialSliders::~PartialSliders()
{
    delete(gainSliderAttachment);
    delete(phaseSliderAttachment);
    delete(gainSlider);
    delete(phaseSlider);
}

void PartialSliders::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(HEIGHT_PARTIAL_GAIN + HEIGHT_PARTIAL_PHASE);

    g.setColour(findColour(juce::Slider::textBoxTextColourId));
    g.setFont(15.0f);
    g.drawFittedText("#" + juce::String(partialIndex + 1), bounds, juce::Justification::centred, 1);

    g.setColour(getLookAndFeel().findColour(juce::GroupComponent::outlineColourId));
    g.drawRect(bounds, 1.f);
}

void PartialSliders::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromBottom(bounds.getHeight() - HEIGHT_PARTIAL_GAIN);
    gainSlider->setBounds(bounds);

    bounds = getLocalBounds();
    bounds.removeFromTop(HEIGHT_PARTIAL_GAIN);
    bounds.removeFromBottom(bounds.getHeight() - HEIGHT_PARTIAL_PHASE);
    phaseSlider->setBounds(bounds);
}
