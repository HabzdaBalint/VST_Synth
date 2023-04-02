/*
==============================================================================

    WaveformSelectorItems.h
    Created: 1 Apr 2023 11:37:29pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

class WaveformMenuItemButton : public juce::Button
{
public:
    WaveformMenuItemButton(const juce::String& buttonName, std::function<void(juce::Graphics&)> newDrawWaveform, std::function<void()> newOnClick) :
        juce::Button(buttonName)
    {  
        drawWaveform = std::move(newDrawWaveform);
        onClick = std::move(newOnClick);
    }

    ~WaveformMenuItemButton() override {}

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = getLocalBounds();

        if( shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown )
            g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
        else
            g.setColour(findColour(juce::PopupMenu::backgroundColourId));

        g.fillAll();

        if( shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown )
            g.setColour(findColour(juce::PopupMenu::highlightedTextColourId));
        else
            g.setColour(findColour(juce::PopupMenu::textColourId));

        drawWaveform(g);
    }

private:
    std::function<void(juce::Graphics&)> drawWaveform;
};

struct WaveformMenuItem : juce::PopupMenu::CustomComponent
{
    WaveformMenuItem(int width, int height, const juce::String& buttonName, std::function<void()> newOnClick, std::function<void(juce::Graphics&)> drawWaveform) :
            juce::PopupMenu::CustomComponent(true),
            waveformButton(buttonName, drawWaveform, newOnClick),
            idealWidth(width),
            idealHeight(height)
    {
        addMouseListener(&waveformButton, true);
        addAndMakeVisible(waveformButton);
    }

    ~WaveformMenuItem()
    {
        removeMouseListener(&waveformButton);
    }

    void getIdealSize (int& width, int& height) override
    {
        width = idealWidth;
        height = idealHeight;
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        //bounds.reduce(PADDING_PX, PADDING_PX);
        waveformButton.setBounds(bounds);
        waveformButton.repaint();
    }

private:
    int idealWidth;
    int idealHeight;
protected:
    WaveformMenuItemButton waveformButton;
};

struct WaveformSine : WaveformMenuItem
{
    WaveformSine(VST_SynthAudioProcessor& p, int width, int height, const juce::String& buttonName) :
        audioProcessor(p),
        WaveformMenuItem( width, height, buttonName,
            [&] () { loadSine(); },
            [&] ( juce::Graphics& g ) { drawSine(g); } )
    {}

    ~WaveformSine() override {}

    void loadSine()
    {
        audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialGainParameterName(0))->setValueNotifyingHost(1);
        audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialPhaseParameterName(0))->setValueNotifyingHost(0);
        for (size_t i = 1; i < HARMONIC_N; i++)
        {
            audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialGainParameterName(i))->setValueNotifyingHost(0);
            audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialPhaseParameterName(i))->setValueNotifyingHost(0);
        }
    }

    void drawSine(juce::Graphics& g)
    {
        auto bounds = waveformButton.getLocalBounds();
        previewPath.clear();

        std::vector<float> amplitudes;
        amplitudes.resize(bounds.getWidth());

        for (size_t i = 0; i < amplitudes.size(); i++)
        {
            amplitudes[i] = sin( juce::jmap( (float)i, 0.f, (float)( amplitudes.size() - 1.f ), 0.f, juce::MathConstants<float>::twoPi ) );
        }

        previewPath.preallocateSpace( 3 * amplitudes.size() );

        bounds.reduce(PADDING_PX, PADDING_PX);

        previewPath.startNewSubPath( bounds.getX(), 
            juce::jmap( -1 * amplitudes.front(), -1.f, 1.f, (float)bounds.getY(), (float)( bounds.getHeight()+bounds.getY() ) ) );

        for (size_t i = 1; i < amplitudes.size(); i++)
        {
            previewPath.lineTo( bounds.getX() + i, juce::jmap( -1 * amplitudes[i], -1.f, 1.f, (float)bounds.getY(), (float)( bounds.getHeight()+bounds.getY() ) ) );
        }

        g.strokePath(previewPath, juce::PathStrokeType(1.f));
    }

private:
    juce::Path previewPath;
    VST_SynthAudioProcessor& audioProcessor;
};

struct WaveformTriangle : WaveformMenuItem
{
    WaveformTriangle(VST_SynthAudioProcessor& p, int width, int height, const juce::String& buttonName) :
        audioProcessor(p),
        WaveformMenuItem( width, height, buttonName,
            [&] () { loadTriange(); },
            [&] ( juce::Graphics& g ) { drawTriangle(g); } )
    {}

    ~WaveformTriangle() override {}

    void loadTriange()
    {
        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialGainParameterName(i))->setValueNotifyingHost(
                ( ( i + 1 ) % 2 == 1 ) ? 1.f / pow( i + 1, 2 ) : 0.f );

            audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialPhaseParameterName(i))->setValueNotifyingHost(
                ( ( i + 1 ) % 4 == 3 ) ? 0.5 : 0.f );
        }
    }

    void drawTriangle(juce::Graphics& g)
    {
        auto bounds = waveformButton.getLocalBounds();
        previewPath.clear();

        bounds.reduce(PADDING_PX, PADDING_PX);

        previewPath.startNewSubPath( bounds.getX(), bounds.getY() + ( bounds.getHeight() / 2.f ) );
        previewPath.lineTo( bounds.getX() + ( bounds.getWidth() / 4.f ), bounds.getY() );
        previewPath.lineTo( bounds.getX() + ( 3.f * bounds.getWidth() / 4.f ), bounds.getY() + bounds.getHeight() );
        previewPath.lineTo( bounds.getX() + bounds.getWidth(), bounds.getY() + ( bounds.getHeight() / 2.f ) );

        g.strokePath(previewPath, juce::PathStrokeType(1.f));
    }

private:
    juce::Path previewPath;
    VST_SynthAudioProcessor& audioProcessor;
};

struct WaveformSquare : WaveformMenuItem
{
    WaveformSquare(VST_SynthAudioProcessor& p, int width, int height, const juce::String& buttonName) :
        audioProcessor(p),
        WaveformMenuItem( width, height, buttonName,
            [&] () { loadSquare(); },
            [&] ( juce::Graphics& g ) { drawSquare(g); } )
    {}

    ~WaveformSquare() override {}

    void loadSquare()
    {
        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialGainParameterName(i))->setValueNotifyingHost(
                ( ( i + 1 ) % 2 == 1 ) ? 1.f / ( i + 1 ) : 0.f );

            audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialPhaseParameterName(i))->setValueNotifyingHost(
                0.f );
        }
    }

    void drawSquare(juce::Graphics& g)
    {
        auto bounds = waveformButton.getLocalBounds();
        previewPath.clear();

        bounds.reduce(PADDING_PX, PADDING_PX);

        previewPath.startNewSubPath( bounds.getX(), bounds.getY() + ( bounds.getHeight() / 2.f ) );
        previewPath.lineTo( bounds.getX(), bounds.getY() );
        previewPath.lineTo( bounds.getX() + ( bounds.getWidth() / 2.f ), bounds.getY() );
        previewPath.lineTo( bounds.getX() + ( bounds.getWidth() / 2.f ), bounds.getY() + bounds.getHeight() );
        previewPath.lineTo( bounds.getX() + bounds.getWidth(), bounds.getY() + bounds.getHeight() );
        previewPath.lineTo( bounds.getX() + bounds.getWidth(), bounds.getY() + ( bounds.getHeight() / 2.f ) );

        g.strokePath(previewPath, juce::PathStrokeType(1.f));
    }

private:
    juce::Path previewPath;
    VST_SynthAudioProcessor& audioProcessor;
};

struct WaveformSawtooth : WaveformMenuItem
{
    WaveformSawtooth(VST_SynthAudioProcessor& p, int width, int height, const juce::String& buttonName) :
        audioProcessor(p),
        WaveformMenuItem( width, height, buttonName,
            [&] () { loadSawtooth(); },
            [&] ( juce::Graphics& g ) { drawSawtooth(g); } )
    {}

    ~WaveformSawtooth() override {}

    void loadSawtooth()
    {
        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialGainParameterName(i))->setValueNotifyingHost(
                1.f / ( i + 1 ) );

            audioProcessor.apvts.getParameter(audioProcessor.additiveSynth->getPartialPhaseParameterName(i))->setValueNotifyingHost(
                ( ( i + 1 ) % 2 == 0 ) ? 0.5 : 0.f );
        }
    }

    void drawSawtooth(juce::Graphics& g)
    {
        auto bounds = waveformButton.getLocalBounds();
        previewPath.clear();

        bounds.reduce(PADDING_PX, PADDING_PX);

        previewPath.startNewSubPath( bounds.getX(), bounds.getY() + ( bounds.getHeight() / 2.f ) );
        previewPath.lineTo( bounds.getX() + ( bounds.getWidth() / 2.f ), bounds.getY() );
        previewPath.lineTo( bounds.getX() + ( bounds.getWidth() / 2.f ), bounds.getY() + bounds.getHeight() );
        previewPath.lineTo( bounds.getX() + bounds.getWidth(), bounds.getY() + ( bounds.getHeight() / 2.f ) );

        g.strokePath(previewPath, juce::PathStrokeType(1.f));
    }

private:
    juce::Path previewPath;
    VST_SynthAudioProcessor& audioProcessor;
};