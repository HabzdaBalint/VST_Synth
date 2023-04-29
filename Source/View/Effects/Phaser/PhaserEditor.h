/*
==============================================================================

    PhaserEditor.h
    Created: 6 Apr 2023 3:10:45pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../../PluginProcessor.h"
#include "../../EditorParameters.h"

#include "../EffectEditor.h"

namespace Editor::Effects
{
    class PhaserEditor : public EffectEditor
    {
    public:
        PhaserEditor(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
        {
            mixKnob = std::make_unique<juce::Slider>(
                juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                juce::Slider::TextEntryBoxPosition::TextBoxBelow);
            mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts,
                "phaserMix",
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
                "phaserRate",
                *rateKnob);
            rateKnob->setScrollWheelEnabled(false);
            rateKnob->setTextValueSuffix(" Hz");
            rateKnob->setTextBoxIsEditable(true);
            addAndMakeVisible(*rateKnob);
            rateLabel = std::make_unique<juce::Label>();
            rateLabel->setText("Rate", juce::NotificationType::dontSendNotification);
            rateLabel->setJustificationType(juce::Justification::centred);
            addAndMakeVisible(*rateLabel);

            depthKnob = std::make_unique<juce::Slider>(
                juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                juce::Slider::TextEntryBoxPosition::TextBoxBelow);
            depthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts,
                "phaserDepth",
                *depthKnob);
            depthKnob->setScrollWheelEnabled(false);
            depthKnob->setTextValueSuffix("%");
            depthKnob->setTextBoxIsEditable(true);
            addAndMakeVisible(*depthKnob);
            depthLabel = std::make_unique<juce::Label>();
            depthLabel->setText("Depth", juce::NotificationType::dontSendNotification);
            depthLabel->setJustificationType(juce::Justification::centred);
            addAndMakeVisible(*depthLabel);

            frequencyKnob = std::make_unique<juce::Slider>(
                juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                juce::Slider::TextEntryBoxPosition::TextBoxBelow);
            frequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts,
                "phaserFrequency",
                *frequencyKnob);
            frequencyKnob->setScrollWheelEnabled(false);
            frequencyKnob->setTextValueSuffix(" Hz");
            frequencyKnob->setTextBoxIsEditable(true);
            addAndMakeVisible(*frequencyKnob);
            frequencyLabel = std::make_unique<juce::Label>();
            frequencyLabel->setText("Frequency", juce::NotificationType::dontSendNotification);
            frequencyLabel->setJustificationType(juce::Justification::centred);
            addAndMakeVisible(*frequencyLabel);

            feedbackKnob = std::make_unique<juce::Slider>(
                juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                juce::Slider::TextEntryBoxPosition::TextBoxBelow);
            feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts,
                "phaserFeedback",
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
            nameLabel->setText("Phaser", juce::NotificationType::dontSendNotification);
            nameLabel->setFont(juce::Font(20));
            nameLabel->setJustificationType(juce::Justification::centredLeft);
            addAndMakeVisible(*nameLabel);
        }

        ~PhaserEditor() override {}

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
                
                juce::GridItem( *depthKnob ).withColumn( { 3 } ).withRow( { 2 } ),
                juce::GridItem( *depthLabel ).withColumn( { 3 } ).withRow( { 3 } ),

                juce::GridItem( *frequencyKnob ).withColumn( { 4 } ).withRow( { 2 } ),
                juce::GridItem( *frequencyLabel ).withColumn( { 4 } ).withRow( { 3 } ),

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

        std::unique_ptr<juce::Slider> mixKnob, rateKnob, depthKnob, frequencyKnob, feedbackKnob;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment, rateAttachment, depthAttachment, frequencyAttachment, feedbackAttachment;
        std::unique_ptr<juce::Label> mixLabel, rateLabel, depthLabel, frequencyLabel, feedbackLabel, nameLabel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaserEditor)
    };
}