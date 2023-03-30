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
    addAndMakeVisible(*waveformViewer);
    waveformEditorViewport->setViewedComponent(new WaveformEditor(p), true);
    addAndMakeVisible(*waveformEditorViewport);
}

OscillatorEditor::~OscillatorEditor() {}

void OscillatorEditor::paint (juce::Graphics& g)
{
    auto bounds = waveformViewer->getBounds();
    g.setColour(getLookAndFeel().findColour(juce::GroupComponent::outlineColourId));
    g.drawRect(bounds, 1.f);

    bounds = waveformEditorViewport->getBounds();
    g.drawRect(bounds, 1.f);
}

void OscillatorEditor::resized()
{
    auto bounds = getLocalBounds().removeFromTop(getLocalBounds().getHeight() * HEIGHT_WAVEFORM_VIEWER_RELATIVE);
    bounds.reduce(PADDING_PX, PADDING_PX);
    waveformViewer->setBounds(bounds);

    bounds = getLocalBounds().removeFromBottom(getLocalBounds().getHeight() * HEIGHT_WAVEFORM_EDITOR_RELATIVE);
    bounds.reduce(PADDING_PX, PADDING_PX);
    waveformEditorViewport->setBounds(bounds);

    bounds = waveformEditorViewport->getViewedComponent()->getBounds();
    bounds.setHeight(waveformEditorViewport->getLocalBounds().getHeight() - 8);
    waveformEditorViewport->getViewedComponent()->setBounds(bounds);
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
    auto bounds = getLocalBounds().removeFromTop(getLocalBounds().getHeight() / 2);
    juce::Line<float> line(bounds.getBottomLeft().toFloat(), bounds.getBottomRight().toFloat());
    g.setColour(findColour(juce::GroupComponent::outlineColourId));
    g.drawLine(line, 1.5);

    g.setColour(findColour(juce::Slider::textBoxTextColourId));
    g.strokePath(waveformPath, juce::PathStrokeType(1.5));
}

void WaveformViewer::resized()
{
    redrawPath();
    repaint();
}

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
        bounds.reduce(0, PADDING_PX);

        waveformPath.scaleToFit(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), false);
    }
    else
    {
        waveformPath.clear();
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
    bounds.setWidth(HARMONIC_N * WIDTH_PARTIAL_SLIDERS_PX);
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
        bounds.setWidth(WIDTH_PARTIAL_SLIDERS_PX);
        bounds.setX(i * WIDTH_PARTIAL_SLIDERS_PX);
        partialSliders[i]->setBounds(bounds);
    }
}

//=========================================================================================================

PartialSliders::PartialSliders(VST_SynthAudioProcessor& p, int idx) : audioProcessor(p), partialIndex(idx)
{
    gainSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::LinearBarVertical,
                                  juce::Slider::TextEntryBoxPosition::TextBoxBelow);
    gainSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,
                                                        audioProcessor.additiveSynth->getPartialGainParameterName(partialIndex),
                                                        *gainSlider);
    gainSlider->setScrollWheelEnabled(false);
    gainSlider->setTextValueSuffix("%");
    gainSlider->setTextBoxIsEditable(false);
    addAndMakeVisible(*gainSlider);

    phaseSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::LinearBar,
                                   juce::Slider::TextEntryBoxPosition::TextBoxBelow);
    phaseSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,
                                                        audioProcessor.additiveSynth->getPartialPhaseParameterName(partialIndex),
                                                        *phaseSlider);    
    phaseSlider->setScrollWheelEnabled(false);
    phaseSlider->setTextValueSuffix("%");
    phaseSlider->setTextBoxIsEditable(false);
    addAndMakeVisible(*phaseSlider);
}

PartialSliders::~PartialSliders() {}

void PartialSliders::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().removeFromBottom( HEIGHT_PARTIAL_NUMBER_PX );

    g.setColour(findColour(juce::Slider::textBoxTextColourId));
    g.setFont(15.0f);
    g.drawFittedText("#" + juce::String(partialIndex + 1), bounds, juce::Justification::centred, 1);

    g.setColour(getLookAndFeel().findColour(juce::GroupComponent::outlineColourId));
    g.drawRect(bounds, 1.f);
}

void PartialSliders::resized()
{
    auto bounds = getLocalBounds().removeFromTop( getLocalBounds().getHeight() - ( HEIGHT_PARTIAL_NUMBER_PX + HEIGHT_PARTIAL_PHASE_PX ) );
    gainSlider->setBounds(bounds);

    bounds = getLocalBounds().removeFromTop( gainSlider->getBounds().getHeight() + HEIGHT_PARTIAL_PHASE_PX ).removeFromBottom( HEIGHT_PARTIAL_PHASE_PX );
    phaseSlider->setBounds(bounds);
}
