/*
==============================================================================

    DelayEditor.h
    Created: 6 Apr 2023 2:41:35pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../../PluginProcessor.h"
#include "../../EditorParameters.h"

#include "../EffectEditor.h"

class DelayEditor : public EffectEditor
{
public:
    DelayEditor(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
    {
        mixKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "delayMix",
            *mixKnob);
        mixKnob->setScrollWheelEnabled(false);
        mixKnob->setTextValueSuffix("%");
        mixKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*mixKnob);
        mixLabel = std::make_unique<juce::Label>();
        mixLabel->setText("Mix", juce::NotificationType::dontSendNotification);
        mixLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*mixLabel);

        feedbackKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "delayFeedback",
            *feedbackKnob);
        feedbackKnob->setScrollWheelEnabled(false);
        feedbackKnob->setTextValueSuffix("%");
        feedbackKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*feedbackKnob);
        feedbackLabel = std::make_unique<juce::Label>();
        feedbackLabel->setText("Feedback", juce::NotificationType::dontSendNotification);
        feedbackLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*feedbackLabel);

        timeKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        timeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "delayTime",
            *timeKnob);
        timeKnob->setScrollWheelEnabled(false);
        timeKnob->setTextValueSuffix("ms");
        timeKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*timeKnob);
        timeLabel = std::make_unique<juce::Label>();
        timeLabel->setText("Time", juce::NotificationType::dontSendNotification);
        timeLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*timeLabel);

        frequencyKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        frequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "delayFilterFrequency",
            *frequencyKnob);
        frequencyKnob->setScrollWheelEnabled(false);
        frequencyKnob->setTextValueSuffix(" Hz");
        frequencyKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*frequencyKnob);
        frequencyLabel = std::make_unique<juce::Label>();
        frequencyLabel->setText("Center Frequency", juce::NotificationType::dontSendNotification);
        frequencyLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*frequencyLabel);

        qKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        qAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "delayFilterQ",
            *qKnob);
        qKnob->setScrollWheelEnabled(false);
        qKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*qKnob);
        qLabel = std::make_unique<juce::Label>();
        qLabel->setText("Q", juce::NotificationType::dontSendNotification);
        qLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*qLabel);

        nameLabel = std::make_unique<juce::Label>();
        nameLabel->setText("Delay", juce::NotificationType::dontSendNotification);
        nameLabel->setFont(juce::Font(20));
        nameLabel->setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(*nameLabel);
    }

    ~DelayEditor() override {}

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
            juce::GridItem( *mixKnob ).withColumn( { 1 } ).withRow( { 2 } ),
            juce::GridItem( *mixLabel ).withColumn( { 1 } ).withRow( { 3 } ),

            juce::GridItem( *feedbackKnob ).withColumn( { 2 } ).withRow( { 2 } ),
            juce::GridItem( *feedbackLabel ).withColumn( { 2 } ).withRow( { 3 } ),
            
            juce::GridItem( *timeKnob ).withColumn( { 3 } ).withRow( { 2 } ),
            juce::GridItem( *timeLabel ).withColumn( { 3 } ).withRow( { 3 } ),

            juce::GridItem( *frequencyKnob ).withColumn( { 4 } ).withRow( { 2 } ),
            juce::GridItem( *frequencyLabel ).withColumn( { 4 } ).withRow( { 3 } ),

            juce::GridItem( *qKnob ).withColumn( { 5 } ).withRow( { 2 } ),
            juce::GridItem( *qLabel ).withColumn( { 5 } ).withRow( { 3 } ),

            juce::GridItem( *nameLabel ).withColumn( { 1, 6 } ).withRow( { 1 } ) };

        grid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        grid.performLayout(bounds);
    }

    const int getIdealHeight() override
    {
        return 150  + 2 * PADDING_PX;
    }
    
private:
    juce::AudioProcessorValueTreeState& apvts;

    std::unique_ptr<juce::Slider> mixKnob, feedbackKnob, timeKnob, frequencyKnob, qKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment, feedbackAttachment, timeAttachment, frequencyAttachment, qAttachment;
    std::unique_ptr<juce::Label> mixLabel, feedbackLabel, timeLabel, frequencyLabel, qLabel, nameLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayEditor)
};