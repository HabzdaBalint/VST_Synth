/*
==============================================================================

    UnisonComponent.h
    Created: 1 Apr 2023 12:29:29am
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

class UnisonComponent : public juce::Component
{
public:
    UnisonComponent(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        unisonCountKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        unisonCountKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "unisonCount",
            *unisonCountKnob);
        unisonCountKnob->setScrollWheelEnabled(false);
        unisonCountKnob->setTextValueSuffix("");
        unisonCountKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*unisonCountKnob);

        unisonCountLabel.setText("Unison Pair Count", juce::NotificationType::dontSendNotification);
        unisonCountLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(unisonCountLabel);

        unisonDetuneKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        unisonDetuneKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "unisonDetune",
            *unisonDetuneKnob);
        unisonDetuneKnob->setScrollWheelEnabled(false);
        unisonDetuneKnob->setTextValueSuffix(" cents");
        unisonDetuneKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*unisonDetuneKnob);

        unisonDetuneLabel.setText("Unison Detune", juce::NotificationType::dontSendNotification);
        unisonDetuneLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(unisonDetuneLabel);

        unisonGainKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        unisonGainKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "unisonGain",
            *unisonGainKnob);
        unisonGainKnob->setScrollWheelEnabled(false);
        unisonGainKnob->setTextValueSuffix("%");
        unisonGainKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*unisonGainKnob);

        unisonGainLabel.setText("Unison Gain", juce::NotificationType::dontSendNotification);
        unisonGainLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(unisonGainLabel);
    }

    ~UnisonComponent() override
    {

    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        g.setColour(findColour(juce::GroupComponent::outlineColourId));
        g.drawRect(bounds, 1.f);
    }

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid unisonComponentGrid;
        unisonComponentGrid.templateRows = { TrackInfo( Fr( 1 ) ), TrackInfo( Px( HEIGHT_UNISON_KNOB_PX ) ), TrackInfo( Px( LABEL_HEIGHT ) ), TrackInfo( Fr( 1 ) ) };
        unisonComponentGrid.templateColumns = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
        unisonComponentGrid.items = {
            juce::GridItem( nullptr ), juce::GridItem( nullptr ), juce::GridItem( nullptr ),
            juce::GridItem( *unisonCountKnob ), juce::GridItem( *unisonDetuneKnob ), juce::GridItem( *unisonGainKnob ),
            juce::GridItem( unisonCountLabel ), juce::GridItem( unisonDetuneLabel ), juce::GridItem( unisonGainLabel ),
            juce::GridItem( nullptr ), juce::GridItem( nullptr ), juce::GridItem( nullptr ) };

        unisonComponentGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        unisonComponentGrid.performLayout(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<juce::Slider> unisonCountKnob;
    std::unique_ptr<juce::Slider> unisonDetuneKnob;
    std::unique_ptr<juce::Slider> unisonGainKnob;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> unisonCountKnobAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> unisonDetuneKnobAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> unisonGainKnobAttachment;

    juce::Label unisonCountLabel;
    juce::Label unisonDetuneLabel;
    juce::Label unisonGainLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UnisonComponent)
};