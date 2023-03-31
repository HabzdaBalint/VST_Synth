/*
==============================================================================

    PartialSlider.h
    Created: 31 Mar 2023 8:00:34pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

class PartialSlider : public juce::Component
{
public:
    PartialSlider(VST_SynthAudioProcessor& p, int idx) : audioProcessor(p), partialIndex(idx)
    {
        gainSlider = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::LinearBarVertical,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        gainSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            audioProcessor.additiveSynth->getPartialGainParameterName(partialIndex),
            *gainSlider);
        gainSlider->setScrollWheelEnabled(false);
        gainSlider->setTextValueSuffix("%");
        gainSlider->setTextBoxIsEditable(false);
        addAndMakeVisible(*gainSlider);

        phaseSlider = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::LinearBar,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        phaseSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            audioProcessor.additiveSynth->getPartialPhaseParameterName(partialIndex),
            *phaseSlider);    
        phaseSlider->setScrollWheelEnabled(false);
        phaseSlider->setTextValueSuffix("%");
        phaseSlider->setTextBoxIsEditable(false);
        addAndMakeVisible(*phaseSlider);
    }

    ~PartialSlider() override {};

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().removeFromBottom( HEIGHT_PARTIAL_NUMBER_PX );

        g.setColour(findColour(juce::Slider::textBoxTextColourId));
        g.setFont(15.0f);
        g.drawFittedText("#" + juce::String(partialIndex + 1), bounds, juce::Justification::centred, 1);

        g.setColour(findColour(juce::GroupComponent::outlineColourId));
        g.drawRect(bounds, 1.f);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().removeFromTop( getLocalBounds().getHeight() - ( HEIGHT_PARTIAL_NUMBER_PX + HEIGHT_PARTIAL_PHASE_PX ) );
        gainSlider->setBounds(bounds);

        bounds = getLocalBounds().removeFromTop( gainSlider->getBounds().getHeight() + HEIGHT_PARTIAL_PHASE_PX ).removeFromBottom( HEIGHT_PARTIAL_PHASE_PX );
        phaseSlider->setBounds(bounds);
    }
    
private:
    VST_SynthAudioProcessor& audioProcessor;

    int partialIndex;

    std::unique_ptr<juce::Slider> gainSlider;
    std::unique_ptr<juce::Slider> phaseSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> phaseSliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PartialSlider)
};