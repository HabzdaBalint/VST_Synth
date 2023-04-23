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

        unisonCountLabel = std::make_unique<juce::Label>();
        unisonCountLabel->setText("Unison Pair Count", juce::NotificationType::dontSendNotification);
        unisonCountLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*unisonCountLabel);

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
        
        unisonDetuneLabel = std::make_unique<juce::Label>();
        unisonDetuneLabel->setText("Unison Detune", juce::NotificationType::dontSendNotification);
        unisonDetuneLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*unisonDetuneLabel);

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

        unisonGainLabel = std::make_unique<juce::Label>();
        unisonGainLabel->setText("Unison Gain", juce::NotificationType::dontSendNotification);
        unisonGainLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*unisonGainLabel);
    }

    ~UnisonComponent() override {}

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid grid;
        grid.templateRows = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 8 ) ), TrackInfo( Px( LABEL_HEIGHT ) ), TrackInfo( Fr( 1 ) ) };
        grid.templateColumns = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
        grid.items = {
            juce::GridItem( *unisonCountKnob ).withColumn( { 1 } ).withRow( { 2, 3 } ),
            juce::GridItem( *unisonCountLabel ).withColumn( { 1 } ).withRow( { 3 } ),
            juce::GridItem( *unisonDetuneKnob ).withColumn( { 2 } ).withRow( { 2, 3 } ),
            juce::GridItem( *unisonDetuneLabel ).withColumn( { 2 } ).withRow( { 3 } ),
            juce::GridItem( *unisonGainKnob ).withColumn( { 3 } ).withRow( { 2, 3 } ),
            juce::GridItem( *unisonGainLabel ).withColumn( { 3 } ).withRow( { 3 } ) };

        grid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        grid.performLayout(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<juce::Slider> unisonCountKnob, unisonDetuneKnob, unisonGainKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> unisonCountKnobAttachment, unisonDetuneKnobAttachment, unisonGainKnobAttachment;

    std::unique_ptr<juce::Label> unisonCountLabel, unisonDetuneLabel, unisonGainLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UnisonComponent)
};