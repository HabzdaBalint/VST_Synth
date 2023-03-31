/*
==============================================================================

    GainKnob.h
    Created: 31 Mar 2023 8:52:30pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

class GainKnob : public juce::Component
{
public:
    GainKnob(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        gainKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::Rotary,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        gainKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "synthGain",
            *gainKnob);
        gainKnob->setScrollWheelEnabled(false);
        gainKnob->setTextValueSuffix(" dB");
        gainKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*gainKnob);
    }

    ~GainKnob() override {}

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        gainKnob->setBounds(bounds);
    }
    
private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<juce::Slider> gainKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainKnobAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainKnob)
};