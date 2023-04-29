/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

==============================================================================
*/

#pragma once

#include "PluginEditor.h"

namespace Editor
{
    VST_SynthAudioProcessorEditor::VST_SynthAudioProcessorEditor (VST_SynthAudioProcessor& p)
        : AudioProcessorEditor (&p), audioProcessor (p)
    {
        setSize(WIDTH_MAIN_WINDOW_PX, HEIGHT_MAIN_WINDOW_PX);
        setResizable(false, true);
        setResizeLimits(WIDTH_MAIN_WINDOW_PX, HEIGHT_MAIN_WINDOW_PX, 1.3 * WIDTH_MAIN_WINDOW_PX, 1.3 * HEIGHT_MAIN_WINDOW_PX);

        addAndMakeVisible(*tabbedComponent);

        keyboardComponent->setKeyPressBaseOctave(4);
        keyboardComponent->setOctaveForMiddleC(4);
        keyboardComponent->setScrollButtonsVisible(false);
        addAndMakeVisible(*keyboardComponent);

        setLookAndFeel(&lnf);    
    }

    VST_SynthAudioProcessorEditor::~VST_SynthAudioProcessorEditor()
    {
        setLookAndFeel(nullptr);
    }

    void VST_SynthAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll( getLookAndFeel().findColour( juce::ResizableWindow::backgroundColourId ) );
    }

    void VST_SynthAudioProcessorEditor::resized()
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid grid;
        grid.templateRows = { TrackInfo( Fr( 5 ) ), TrackInfo( Fr( 1 ) ) };
        grid.templateColumns= { TrackInfo( Fr( 1 ) ) };
        grid.items = {
            juce::GridItem( *tabbedComponent ).withColumn( { 1 } ).withRow( { 1 } ),
            juce::GridItem( *keyboardComponent ).withColumn( { 1 } ).withRow( { 2 } ) };

        auto bounds = getLocalBounds();
        grid.performLayout(bounds);

        keyboardComponent->setKeyWidth( keyboardComponent->getWidth() / ( 35.f + ( keyboardComponent->getWidth() > 1.2 * WIDTH_MAIN_WINDOW_PX ? 7.f : 0.f ) ) );
        keyboardComponent->setAvailableRange(36 - ( keyboardComponent->getWidth() > 1.2 * WIDTH_MAIN_WINDOW_PX ? 12 : 0 ), 95);
    }

    VST_SynthTabbedComponent::VST_SynthTabbedComponent(VST_SynthAudioProcessor& p)
        : audioProcessor (p),
        TabbedComponent(juce::TabbedButtonBar::TabsAtTop)
    {
        auto color = findColour(juce::ResizableWindow::backgroundColourId);

        addTab("Oscillator", color, new Oscillator::OscillatorTab(p), true);
        addTab("Synthesizer", color, new Synthesizer::SynthesizerTab(p), true);
        addTab("Effects", color, new Effects::EffectsTab(p), true);

        setOrientation(juce::TabbedButtonBar::Orientation::TabsAtTop);
        setCurrentTabIndex(1);
        setTabBarDepth(35);
    }

    VST_SynthTabbedComponent::~VST_SynthTabbedComponent() {}

    void VST_SynthTabbedComponent::lookAndFeelChanged()
    {
        auto color = findColour(juce::ResizableWindow::backgroundColourId);
        for (size_t i = 0; i < getNumTabs(); i++)
        {
            setTabBackgroundColour(i, color);
        }
    }
}