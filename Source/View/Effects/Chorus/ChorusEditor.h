/*
==============================================================================

    ChorusEditor.h
    Created: 5 Apr 2023 4:38:09pm
    Author:  habzd

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../../PluginProcessor.h"
#include "../../EditorParameters.h"

#include "../EffectEditor.h"

class ChorusEditor : public EffectEditor
{
public:
    ChorusEditor(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
    {
        mixKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "chorusMix",
            *mixKnob);
        mixKnob->setScrollWheelEnabled(false);
        mixKnob->setTextValueSuffix("%");
        mixKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*mixKnob);
        mixLabel = std::make_unique<juce::Label>();
        mixLabel->setText("Mix", juce::NotificationType::dontSendNotification);
        mixLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*mixLabel);

        rateKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        rateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "chorusRate",
            *rateKnob);
        rateKnob->setScrollWheelEnabled(false);
        rateKnob->setTextValueSuffix(" Hz");
        rateKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*rateKnob);
        rateLabel = std::make_unique<juce::Label>();
        rateLabel->setText("Rate", juce::NotificationType::dontSendNotification);
        rateLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*rateLabel);

        delayKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        delayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "chorusDelay",
            *delayKnob);
        delayKnob->setScrollWheelEnabled(false);
        delayKnob->setTextValueSuffix(" ms");
        delayKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*delayKnob);
        delayLabel = std::make_unique<juce::Label>();
        delayLabel->setText("Delay", juce::NotificationType::dontSendNotification);
        delayLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*delayLabel);

        depthKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        depthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "chorusDepth",
            *depthKnob);
        depthKnob->setScrollWheelEnabled(false);
        depthKnob->setTextValueSuffix("%");
        depthKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*depthKnob);
        depthLabel = std::make_unique<juce::Label>();
        depthLabel->setText("Depth", juce::NotificationType::dontSendNotification);
        depthLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*depthLabel);

        feedbackKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "chorusFeedback",
            *feedbackKnob);
        feedbackKnob->setScrollWheelEnabled(false);
        feedbackKnob->setTextValueSuffix("%");
        feedbackKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*feedbackKnob);
        feedbackLabel = std::make_unique<juce::Label>();
        feedbackLabel->setText("Feedback", juce::NotificationType::dontSendNotification);
        feedbackLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*feedbackLabel);

        nameLabel = std::make_unique<juce::Label>();
        nameLabel->setText("Chorus", juce::NotificationType::dontSendNotification);
        nameLabel->setFont(juce::Font(20));
        nameLabel->setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(*nameLabel);
    }

    ~ChorusEditor() override {}

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

            juce::GridItem( *rateKnob ).withColumn( { 2 } ).withRow( { 2 } ),
            juce::GridItem( *rateLabel ).withColumn( { 2 } ).withRow( { 3 } ),

            juce::GridItem( *delayKnob ).withColumn( { 3 } ).withRow( { 2 } ),
            juce::GridItem( *delayLabel ).withColumn( { 3 } ).withRow( { 3 } ),

            juce::GridItem( *depthKnob ).withColumn( { 4 } ).withRow( { 2 } ),
            juce::GridItem( *depthLabel ).withColumn( { 4 } ).withRow( { 3 } ), 

            juce::GridItem( *feedbackKnob ).withColumn( { 5 } ).withRow( { 2 } ),
            juce::GridItem( *feedbackLabel ).withColumn( { 5 } ).withRow( { 3 } ),

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

    std::unique_ptr<juce::Slider> mixKnob, rateKnob, delayKnob, depthKnob, feedbackKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment, rateAttachment, delayAttachment, depthAttachment, feedbackAttachment;
    std::unique_ptr<juce::Label> mixLabel, rateLabel, delayLabel, depthLabel, feedbackLabel, nameLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChorusEditor)
};