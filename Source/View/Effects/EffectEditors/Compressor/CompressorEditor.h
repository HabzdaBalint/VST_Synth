/*
==============================================================================

    CompressorEditor.h
    Created: 6 Apr 2023 2:21:34pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../../../PluginProcessor.h"
#include "../../../EditorParameters.h"

#include "../EffectEditorUnit.h"

class CompressorEditor : public EffectEditorUnit
{
public:
    CompressorEditor(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
    {
        mixKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "compressorMix",
            *mixKnob);
        mixKnob->setScrollWheelEnabled(false);
        mixKnob->setTextValueSuffix("%");
        mixKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*mixKnob);
        mixLabel = std::make_unique<juce::Label>();
        mixLabel->setText("Mix", juce::NotificationType::dontSendNotification);
        mixLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*mixLabel);

        thresholdKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "compressorThreshold",
            *thresholdKnob);
        thresholdKnob->setScrollWheelEnabled(false);
        thresholdKnob->setTextValueSuffix(" dB");
        thresholdKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*thresholdKnob);
        thresholdLabel = std::make_unique<juce::Label>();
        thresholdLabel->setText("Threshold", juce::NotificationType::dontSendNotification);
        thresholdLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*thresholdLabel);

        ratioKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "compressorRatio",
            *ratioKnob);
        ratioKnob->setScrollWheelEnabled(false);
        ratioKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*ratioKnob);
        ratioLabel = std::make_unique<juce::Label>();
        ratioLabel->setText("Ratio", juce::NotificationType::dontSendNotification);
        ratioLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*ratioLabel);

        attackKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "compressorAttack",
            *attackKnob);
        attackKnob->setScrollWheelEnabled(false);
        attackKnob->setTextValueSuffix(" ms");
        attackKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*attackKnob);
        attackLabel = std::make_unique<juce::Label>();
        attackLabel->setText("Attack", juce::NotificationType::dontSendNotification);
        attackLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*attackLabel);

        releaseKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts,
            "compressorRelease",
            *releaseKnob);
        releaseKnob->setScrollWheelEnabled(false);
        releaseKnob->setTextValueSuffix(" ms");
        releaseKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*releaseKnob);
        releaseLabel = std::make_unique<juce::Label>();
        releaseLabel->setText("Release", juce::NotificationType::dontSendNotification);
        releaseLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*releaseLabel);

        nameLabel = std::make_unique<juce::Label>();
        nameLabel->setText("Compressor", juce::NotificationType::dontSendNotification);
        nameLabel->setFont(juce::Font(20));
        nameLabel->setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(*nameLabel);
    }

    ~CompressorEditor() override {}

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

            juce::GridItem( *thresholdKnob ).withColumn( { 2 } ).withRow( { 2 } ),
            juce::GridItem( *thresholdLabel ).withColumn( { 2 } ).withRow( { 3 } ),
            
            juce::GridItem( *ratioKnob ).withColumn( { 3 } ).withRow( { 2 } ),
            juce::GridItem( *ratioLabel ).withColumn( { 3 } ).withRow( { 3 } ),

            juce::GridItem( *attackKnob ).withColumn( { 4 } ).withRow( { 2 } ),
            juce::GridItem( *attackLabel ).withColumn( { 4 } ).withRow( { 3 } ),

            juce::GridItem( *releaseKnob ).withColumn( { 5 } ).withRow( { 2 } ),
            juce::GridItem( *releaseLabel ).withColumn( { 5 } ).withRow( { 3 } ),

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

    std::unique_ptr<juce::Slider> mixKnob, thresholdKnob, ratioKnob, attackKnob, releaseKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment, thresholdAttachment, ratioAttachment, attackAttachment, releaseAttachment;
    std::unique_ptr<juce::Label> mixLabel, thresholdLabel, ratioLabel, attackLabel, releaseLabel, nameLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorEditor)
};