/*
==============================================================================

    ReverbEditor.h
    Created: 6 Apr 2023 3:16:48pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../../PluginProcessor.h"
#include "../../EditorParameters.h"

#include "../EffectEditor.h"

class ReverbEditor : public EffectEditor
{
public:
    ReverbEditor(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
    {
        wetKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        wetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "reverbWet",
            *wetKnob);
        wetKnob->setScrollWheelEnabled(false);
        wetKnob->setTextValueSuffix("%");
        wetKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*wetKnob);
        wetLabel = std::make_unique<juce::Label>();
        wetLabel->setText("Wet", juce::NotificationType::dontSendNotification);
        wetLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*wetLabel);

        dryKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        dryAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "reverbDry",
            *dryKnob);
        dryKnob->setScrollWheelEnabled(false);
        dryKnob->setTextValueSuffix("%");
        dryKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*dryKnob);
        dryLabel = std::make_unique<juce::Label>();
        dryLabel->setText("Dry", juce::NotificationType::dontSendNotification);
        dryLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*dryLabel);

        roomKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        roomAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "reverbRoom",
            *roomKnob);
        roomKnob->setScrollWheelEnabled(false);
        roomKnob->setTextValueSuffix("%");
        roomKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*roomKnob);
        roomLabel = std::make_unique<juce::Label>();
        roomLabel->setText("Room size", juce::NotificationType::dontSendNotification);
        roomLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*roomLabel);

        dampingKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        dampingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "reverbDamping",
            *dampingKnob);
        dampingKnob->setScrollWheelEnabled(false);
        dampingKnob->setTextValueSuffix("%");
        dampingKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*dampingKnob);
        dampingLabel = std::make_unique<juce::Label>();
        dampingLabel->setText("Damping", juce::NotificationType::dontSendNotification);
        dampingLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*dampingLabel);

        widthKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "reverbWidth",
            *widthKnob);
        widthKnob->setScrollWheelEnabled(false);
        widthKnob->setTextValueSuffix("%");
        widthKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*widthKnob);
        widthLabel = std::make_unique<juce::Label>();
        widthLabel->setText("Width", juce::NotificationType::dontSendNotification);
        widthLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*widthLabel);

        nameLabel = std::make_unique<juce::Label>();
        nameLabel->setText("Reverb", juce::NotificationType::dontSendNotification);
        nameLabel->setFont(juce::Font(20));
        nameLabel->setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(*nameLabel);
    }

    ~ReverbEditor() override {}

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid grid;
        grid.templateRows = { TrackInfo( Px( LABEL_HEIGHT ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Px( LABEL_HEIGHT ) ) };
        grid.templateColumns = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
        grid.items = {
            juce::GridItem( *wetKnob ).withColumn( { 1 } ).withRow( { 2 } ),
            juce::GridItem( *wetLabel ).withColumn( { 1 } ).withRow( { 3 } ),

            juce::GridItem( *dryKnob ).withColumn( { 2 } ).withRow( { 2 } ),
            juce::GridItem( *dryLabel ).withColumn( { 2 } ).withRow( { 3 } ),
            
            juce::GridItem( *roomKnob ).withColumn( { 3 } ).withRow( { 2 } ),
            juce::GridItem( *roomLabel ).withColumn( { 3 } ).withRow( { 3 } ),

            juce::GridItem( *dampingKnob ).withColumn( { 4 } ).withRow( { 2 } ),
            juce::GridItem( *dampingLabel ).withColumn( { 4 } ).withRow( { 3 } ),

            juce::GridItem( *widthKnob ).withColumn( { 5 } ).withRow( { 2 } ),
            juce::GridItem( *widthLabel ).withColumn( { 5 } ).withRow( { 3 } ),
            
            juce::GridItem( *nameLabel ).withColumn( { 1, 6 } ).withRow( { 1 } ) };

        grid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        grid.performLayout(bounds);
    }

    const int getIdealHeight() override
    {
        return 150 + 2 * PADDING_PX;
    }

private:
    juce::AudioProcessorValueTreeState& apvts;

    std::unique_ptr<juce::Slider> wetKnob, dryKnob, dampingKnob, roomKnob, widthKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetAttachment, dryAttachment, dampingAttachment, roomAttachment, widthAttachment;
    std::unique_ptr<juce::Label> wetLabel, dryLabel, dampingLabel, roomLabel, widthLabel, nameLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbEditor)
};