/*
==============================================================================

    WaveformSelector.h
    Created: 1 Apr 2023 2:38:37pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "../Oscillator/WaveformViewer.h"
#include "WaveformSelectorItems.h"

class WaveformMenuButton : public juce::Button
{
public:
    WaveformMenuButton(VST_SynthAudioProcessor& p, const juce::String& buttonName) : audioProcessor(p), juce::Button(buttonName)
    {
        onClick = [&] ()
        {
            juce::PopupMenu menu;
            menu.addCustomItem(1, std::make_unique<WaveformSine>(p, WIDTH_WAVEFORM_SELECTOR_PX, HEIGHT_WAVEFORM_SELECTOR_PX, "Sine"), nullptr, "Sine");
            menu.addCustomItem(2, std::make_unique<WaveformTriangle>(p, WIDTH_WAVEFORM_SELECTOR_PX, HEIGHT_WAVEFORM_SELECTOR_PX, "Triangle"), nullptr, "Triangle");
            menu.addCustomItem(3, std::make_unique<WaveformSquare>(p, WIDTH_WAVEFORM_SELECTOR_PX, HEIGHT_WAVEFORM_SELECTOR_PX, "Square"), nullptr, "Square");
            menu.addCustomItem(4, std::make_unique<WaveformSawtooth>(p, WIDTH_WAVEFORM_SELECTOR_PX, HEIGHT_WAVEFORM_SELECTOR_PX, "Sawtooth"), nullptr, "Sawtooth");
            menu.addCustomItem(4, std::make_unique<WaveformSawSquare>(p, WIDTH_WAVEFORM_SELECTOR_PX, HEIGHT_WAVEFORM_SELECTOR_PX, "SawSquare"), nullptr, "SawSquare");

            auto& lnf = getLookAndFeel();
            menu.setLookAndFeel(&lnf);
            menu.showMenuAsync(juce::PopupMenu::Options{}.withTargetComponent(this)
                                                         .withMaximumNumColumns(1));
        };

        waveformViewer->addMouseListener(this, true);
        addAndMakeVisible(*waveformViewer);
    }

    ~WaveformMenuButton() override
    {
        waveformViewer->removeMouseListener(this);
    }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = getLocalBounds();

        auto& lnf = getLookAndFeel();

        if( shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown )
            g.setColour(findColour(juce::Slider::textBoxTextColourId));
        else
            g.setColour(findColour(juce::GroupComponent::outlineColourId));

        g.drawRect(bounds, 1.f);

        bounds.reduce(PADDING_PX, PADDING_PX);
        waveformViewer->setBounds(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<WaveformViewer> waveformViewer = std::make_unique<WaveformViewer>(audioProcessor);
};

class WaveformSelector : public juce::Component
{
public:
    WaveformSelector(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        addAndMakeVisible(*waveformMenuButton);
    }

    ~WaveformSelector() override {}

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        g.setColour(findColour(juce::GroupComponent::outlineColourId));
        g.drawRect(bounds, 1.f);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        waveformMenuButton->setBounds(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<WaveformMenuButton> waveformMenuButton = std::make_unique<WaveformMenuButton>(audioProcessor ,"Waveforms");

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformSelector)
};


