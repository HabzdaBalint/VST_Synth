/*
==============================================================================

    AdditiveSynthesizerEditor.cpp
    Created: 25 Mar 2023 9:08:06pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "AdditiveSynthesizerEditor.h"

AdditiveSynthesizerEditor::AdditiveSynthesizerEditor(AdditiveSynthesizer& p)
    : juce::AudioProcessorEditor(&p), audioProcessor(p)
{
    /*const auto& parameters = juce::AudioProcessorParameterGroup
    for (size_t i = 0; i < HARMONIC_N; i++)
    {

    }*/
    
}

AdditiveSynthesizerEditor::~AdditiveSynthesizerEditor()
{
    setLookAndFeel(nullptr);
}

void AdditiveSynthesizerEditor::paint (juce::Graphics& g)
{
    g.setColour(juce::Colours::white);
    g.strokePath(waveformPath, juce::PathStrokeType(1.f));
}

void AdditiveSynthesizerEditor::resized()
{
   
}

void AdditiveSynthesizerEditor::parameterValueChanged (int parameterIndex, float newValue)
{
    parameterChanged.set(true);
}

void AdditiveSynthesizerEditor::timerCallback()
{
    if( parameterChanged.compareAndSetBool(false, true) )
    {
        redrawPath();
    }
    
    repaint();
}

void AdditiveSynthesizerEditor::redrawPath()
{
    waveformPath.clear();

    std::vector<float> amplitudes;
    amplitudes.resize(LOOKUP_POINTS);

    for (size_t i = 0; i < LOOKUP_POINTS; i++)
    {
        amplitudes[i] = audioProcessor.WaveTableFormula((juce::MathConstants<float>::twoPi * (float)i) / (float)LOOKUP_POINTS, HARMONIC_N);
    }

    waveformPath.preallocateSpace(3*LOOKUP_POINTS);
    waveformPath.startNewSubPath(getX(), amplitudes.front());
    for (size_t i = 1; i < amplitudes.size(); i++)
    {
        waveformPath.lineTo(getX() + i, amplitudes[i]);
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

        waveformPath.scaleToFit(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), false);
    }
    else
    {
        //todo: draw a straight line in the middle
    }
}
