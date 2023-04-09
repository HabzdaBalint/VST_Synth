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
    WaveformMenuItemButton(const juce::String& buttonName, std::function<void()> newOnClick, std::function<void(juce::Graphics&, juce::Rectangle<int>)> newDrawWaveform) :
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

        drawWaveform(g, bounds);
    }

private:
    std::function<void(juce::Graphics&, juce::Rectangle<int>)> drawWaveform;
};

struct PartialState
{
    PartialState() {}

    PartialState(float gain, float phase) : gain(gain), phase(phase) {}

    float gain = 0;
    float phase = 0;
};

class WaveformMenuItem : public juce::PopupMenu::CustomComponent
{
public:
    WaveformMenuItem(VST_SynthAudioProcessor& p, int width, int height, const juce::String& buttonName) :
        juce::PopupMenu::CustomComponent(true),
        audioProcessor(p),
        waveformButton(buttonName, [&](){ loadWaveform(); }, [&]( juce::Graphics& g, juce::Rectangle<int> r ){ drawWaveform(g, r); }),
        idealWidth(width),
        idealHeight(height)
    {
        addMouseListener(&waveformButton, true);
        addAndMakeVisible(waveformButton);
    }

    WaveformMenuItem(VST_SynthAudioProcessor& p, const juce::String& buttonName) :
        juce::PopupMenu::CustomComponent(true),
        audioProcessor(p),
        waveformButton(buttonName, [&](){ loadWaveform(); }, [&]( juce::Graphics& g, juce::Rectangle<int> r ){ drawWaveform(g, r); })
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
        waveformButton.setBounds(bounds);
        waveformButton.repaint();
    }

    void loadWaveform()
    {
        const auto dataset = prepareWaveform();

        for (size_t i = 0; i < Synthesizer::HARMONIC_N; i++)
        {
            if( i >= dataset.size() )
            {
                audioProcessor.apvts.getParameter(Synthesizer::OscillatorParameters::getPartialGainParameterName(i))->setValueNotifyingHost(0.f);
                audioProcessor.apvts.getParameter(Synthesizer::OscillatorParameters::getPartialPhaseParameterName(i))->setValueNotifyingHost(0.f);
            }
            else
            {
                audioProcessor.apvts.getParameter(Synthesizer::OscillatorParameters::getPartialGainParameterName(i))->setValueNotifyingHost(dataset[i].gain);
                audioProcessor.apvts.getParameter(Synthesizer::OscillatorParameters::getPartialPhaseParameterName(i))->setValueNotifyingHost(dataset[i].phase);
            }
        }
    }

    /// @brief Creates a state for a given waveform
    /// @return The state
    virtual const juce::Array<PartialState> prepareWaveform() = 0;

    /// @brief Draws graphics for the custom menu item
    /// @param g Graphics object to draw with
    /// @param r Rectangle that represents the bounds of the menu item
    virtual void drawWaveform(juce::Graphics& g, juce::Rectangle<int> r) = 0;

private:
    VST_SynthAudioProcessor& audioProcessor;
    WaveformMenuItemButton waveformButton;

    int idealWidth = WIDTH_WAVEFORM_SELECTOR_PX;
    int idealHeight = HEIGHT_WAVEFORM_SELECTOR_PX;
};

class WaveformSine : public WaveformMenuItem
{
public:
    WaveformSine(VST_SynthAudioProcessor& p, int width, int height, const juce::String& buttonName) :
        WaveformMenuItem(p, width, height, buttonName )
    {}

    WaveformSine(VST_SynthAudioProcessor& p, const juce::String& buttonName) :
        WaveformMenuItem(p, buttonName )
    {}

    const juce::Array<PartialState> prepareWaveform() override
    {
        juce::Array<PartialState> state;

        //One sine wave with no phase alterations
        state.add( { 1.f, 0.f } );

        return state;
    }

    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> r) override
    {
        previewPath.clear();

        std::vector<float> amplitudes;
        amplitudes.resize(r.getWidth());

        for (size_t i = 0; i < amplitudes.size(); i++)
        {
            amplitudes[i] = sin( juce::jmap( (float)i, 0.f, (float)( amplitudes.size() - 1.f ), 0.f, juce::MathConstants<float>::twoPi ) );
        }

        previewPath.preallocateSpace( 3 * amplitudes.size() );

        r.reduce(PADDING_PX, PADDING_PX);

        previewPath.startNewSubPath( r.getX(), 
            juce::jmap( -1 * amplitudes.front(), -1.f, 1.f, (float)r.getY(), (float)( r.getHeight()+r.getY() ) ) );

        for (size_t i = 1; i < amplitudes.size(); i++)
        {
            previewPath.lineTo( r.getX() + i, juce::jmap( -1 * amplitudes[i], -1.f, 1.f, (float)r.getY(), (float)( r.getHeight()+r.getY() ) ) );
        }

        g.strokePath(previewPath, juce::PathStrokeType(1.5));
    }

private:
    juce::Path previewPath;
};

class WaveformTriangle : public WaveformMenuItem
{
public:
    WaveformTriangle(VST_SynthAudioProcessor& p, int width, int height, const juce::String& buttonName) :
        WaveformMenuItem(p, width, height, buttonName)
    {}

    WaveformTriangle(VST_SynthAudioProcessor& p, const juce::String& buttonName) :
        WaveformMenuItem(p, buttonName)
    {}

    const juce::Array<PartialState> prepareWaveform() override
    {
        juce::Array<PartialState> state;

        //A triangle's partials' gains quickly tend to 0. Only odd harmonics are used, and every second odd harmonic is offset by half its phase
        for (size_t i = 0; i < Synthesizer::HARMONIC_N; i++)
        {
            float gain, phase;
            
            gain = ( ( i + 1 ) % 2 == 1 ) ? 1.f / pow( i + 1, 2 ) : 0.f;

            phase = ( ( i + 1 ) % 4 == 3 ) ? 0.5 : 0.f;

            state.add( { gain, phase } );
        }
        
        return state;
    }

    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> r) override
    {
        previewPath.clear();

        r.reduce(PADDING_PX, PADDING_PX);

        previewPath.startNewSubPath( r.getX(), r.getY() + ( r.getHeight() / 2.f ) );
        previewPath.lineTo( r.getX() + ( r.getWidth() / 4.f ), r.getY() );
        previewPath.lineTo( r.getX() + ( 3.f * r.getWidth() / 4.f ), r.getY() + r.getHeight() );
        previewPath.lineTo( r.getX() + r.getWidth(), r.getY() + ( r.getHeight() / 2.f ) );

        g.strokePath(previewPath, juce::PathStrokeType(1.5));
    }

private:
    juce::Path previewPath;
};

class WaveformSquare : public WaveformMenuItem
{
public:
    WaveformSquare(VST_SynthAudioProcessor& p, int width, int height, const juce::String& buttonName) :
        WaveformMenuItem(p, width, height, buttonName)
    {}

    WaveformSquare(VST_SynthAudioProcessor& p, const juce::String& buttonName) :
        WaveformMenuItem(p, buttonName)
    {}

    const juce::Array<PartialState> prepareWaveform() override
    {
        juce::Array<PartialState> state;

        //A square's partials' gains tend to 0 like the triangle's, but slower. Only odd harmonics are used, all in the same phase
        for (size_t i = 0; i < Synthesizer::HARMONIC_N; i++)
        {
            float gain, phase;
            
            gain = ( ( i + 1 ) % 2 == 1 ) ? 1.f / ( i + 1 ) : 0.f;

            phase = 0.f;

            state.add( { gain, phase } );
        }
        
        return state;
    }

    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> r) override
    {
        previewPath.clear();

        r.reduce(PADDING_PX, PADDING_PX);

        previewPath.startNewSubPath( r.getX(), r.getY() + ( r.getHeight() / 2.f ) );
        previewPath.lineTo( r.getX(), r.getY() );
        previewPath.lineTo( r.getX() + ( r.getWidth() / 2.f ), r.getY() );
        previewPath.lineTo( r.getX() + ( r.getWidth() / 2.f ), r.getY() + r.getHeight() );
        previewPath.lineTo( r.getX() + r.getWidth(), r.getY() + r.getHeight() );
        previewPath.lineTo( r.getX() + r.getWidth(), r.getY() + ( r.getHeight() / 2.f ) );

        g.strokePath(previewPath, juce::PathStrokeType(1.5));
    }

private:
    juce::Path previewPath;
};

class WaveformSawtooth : public WaveformMenuItem
{
public:
    WaveformSawtooth(VST_SynthAudioProcessor& p, int width, int height, const juce::String& buttonName) :
        WaveformMenuItem(p, width, height, buttonName)
    {}

    WaveformSawtooth(VST_SynthAudioProcessor& p, const juce::String& buttonName) :
        WaveformMenuItem(p, buttonName)
    {}

    const juce::Array<PartialState> prepareWaveform() override
    {
        juce::Array<PartialState> state;

        //A sawtooth has every partial in the harmonic series. Every even partial is offset by half its phase
        for (size_t i = 0; i < Synthesizer::HARMONIC_N; i++)
        {
            float gain, phase;
            
            gain = 1.f / ( i + 1 );

            phase =  ( ( i + 1 ) % 2 == 0 ) ? 0.5 : 0.f;

            state.add( { gain, phase } );
        }
        
        return state;
    }

    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> r) override
    {
        previewPath.clear();

        r.reduce(PADDING_PX, PADDING_PX);

        previewPath.startNewSubPath( r.getX(), r.getY() + ( r.getHeight() / 2.f ) );
        previewPath.lineTo( r.getX() + ( r.getWidth() / 2.f ), r.getY() );
        previewPath.lineTo( r.getX() + ( r.getWidth() / 2.f ), r.getY() + r.getHeight() );
        previewPath.lineTo( r.getX() + r.getWidth(), r.getY() + ( r.getHeight() / 2.f ) );

        g.strokePath(previewPath, juce::PathStrokeType(1.5));
    }

private:
    juce::Path previewPath;
};

class WaveformSawSquare : public WaveformMenuItem
{
public:
    WaveformSawSquare(VST_SynthAudioProcessor& p, int width, int height, const juce::String& buttonName) :
        WaveformMenuItem(p, width, height, buttonName)
    {}   
    
    WaveformSawSquare(VST_SynthAudioProcessor& p, const juce::String& buttonName) :
        WaveformMenuItem(p, buttonName)
    {}

    const juce::Array<PartialState> prepareWaveform() override
    {
        juce::Array<PartialState> state;

        //A saw-square has every partial, but the even partials are lower in amplitude. Phases are 0
        for (size_t i = 0; i < Synthesizer::HARMONIC_N; i++)
        {
            float gain, phase;
            
            gain =  ( ( i + 1 ) % 2 == 1 ) ? 1.f / ( i + 1 ) : 1.f / ( 2 * ( i + 1 ) );

            phase = 0.f;

            state.add( { gain, phase } );
        }
        
        return state;
    }

    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> r) override
    {
        previewPath.clear();

        r.reduce(PADDING_PX, PADDING_PX);

        previewPath.startNewSubPath( r.getX(), r.getY() + ( r.getHeight() / 2.f ) );
        previewPath.lineTo( r.getX(), r.getY() );
        previewPath.lineTo( r.getX() + ( r.getWidth() / 2.f ), ( r.getY() + r.getHeight() / 4 ) );
        previewPath.lineTo( r.getX() + ( r.getWidth() / 2.f ), ( r.getY() + 3 * r.getHeight() / 4 ) );
        previewPath.lineTo( r.getX() + r.getWidth(), r.getY() + r.getHeight() );
        previewPath.lineTo( r.getX() + r.getWidth(), r.getY() + ( r.getHeight() / 2.f ) );

        g.strokePath(previewPath, juce::PathStrokeType(1.5));
    }

private:
    juce::Path previewPath;
};

//To add a new waveform, just implement the drawing function and the state preparer