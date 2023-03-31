/*
==============================================================================

    WaveformViewer.h
    Created: 31 Mar 2023 8:00:51pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

class WaveformViewer : public juce::Component,
                       public juce::Timer,
                       public juce::AudioProcessorParameter::Listener
{
public:
    WaveformViewer(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialGainParameterName(i))->addListener(this);
            audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialPhaseParameterName(i))->addListener(this);
        }

        startTimerHz(60);

        redrawPath();
    }

    ~WaveformViewer() override
    {
        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialGainParameterName(i))->removeListener(this);
            audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialPhaseParameterName(i))->removeListener(this);
        }
    }

    void parameterValueChanged (int parameterIndex, float newValue) override
    {
        parameterChanged.set(true);
    }

    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}

    void timerCallback() override
    {
        if( parameterChanged.compareAndSetBool(false, true) )
        {
            redrawPath();
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().removeFromTop(getLocalBounds().getHeight() / 2);
        juce::Line<float> line(bounds.getBottomLeft().toFloat(), bounds.getBottomRight().toFloat());
        g.setColour(findColour(juce::GroupComponent::outlineColourId));
        g.drawLine(line, 1.5);

        g.setColour(findColour(juce::Slider::textBoxTextColourId));
        g.strokePath(waveformPath, juce::PathStrokeType(1.5));
    }

    void resized() override
    {
        redrawPath();
        repaint();
    }
    
private:
    VST_SynthAudioProcessor& audioProcessor;

    juce::Path waveformPath;

    juce::Atomic<bool> parameterChanged { false };

    void redrawPath()
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformViewer)
};