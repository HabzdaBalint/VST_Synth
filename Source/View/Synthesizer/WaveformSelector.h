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

namespace Editor::Synthesizer
{
    class WaveformMenuButton : public juce::Button
    {
    public:
        WaveformMenuButton(VST_SynthAudioProcessor& p, const juce::String& buttonName) : audioProcessor(p), juce::Button(buttonName)
        {
            onClick = [&] ()
            {
                juce::PopupMenu menu;
                menu.addCustomItem(1, std::make_unique<WaveformSine>(p, "Sine"), nullptr, "Sine");
                menu.addCustomItem(2, std::make_unique<WaveformTriangle>(p, "Triangle"), nullptr, "Triangle");
                menu.addCustomItem(3, std::make_unique<WaveformSquare>(p, "Square"), nullptr, "Square");
                menu.addCustomItem(4, std::make_unique<WaveformSawtooth>(p, "Sawtooth"), nullptr, "Sawtooth");
                menu.addCustomItem(4, std::make_unique<WaveformSawSquare>(p, "SawSquare"), nullptr, "SawSquare");

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
            g.setColour(findColour(juce::GroupComponent::outlineColourId));
            auto bounds = waveformViewer->getBounds();
            g.drawRoundedRectangle(bounds.toFloat(), 4.f, OUTLINE_WIDTH);        
        }

        void resized() override
        {
            auto bounds = getLocalBounds();
            bounds.reduce(PADDING_PX, PADDING_PX);
            waveformViewer->setBounds(bounds);
        }

    private:
        VST_SynthAudioProcessor& audioProcessor;

        std::unique_ptr<Editor::Oscillator::WaveformViewer> waveformViewer = std::make_unique<Editor::Oscillator::WaveformViewer>(audioProcessor);
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
            auto state = waveformMenuButton->getState();
            if( state == juce::Button::ButtonState::buttonDown || state == juce::Button::ButtonState::buttonOver )
                g.setColour(findColour(juce::DrawableButton::backgroundOnColourId));
            else
                g.setColour(findColour(juce::GroupComponent::outlineColourId));

            auto bounds = waveformMenuButton->getBounds();
            g.drawRoundedRectangle(bounds.toFloat(), 4.f, OUTLINE_WIDTH);
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
}