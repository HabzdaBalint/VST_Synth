/*
==============================================================================

    FilterEditor.h
    Created: 6 Apr 2023 2:46:46pm
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
    class FilterEditor : public EffectEditor
    {
    public:
        FilterEditor(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
        {
            using namespace Processor::Effects::Filter;

            mixKnob = std::make_unique<juce::Slider>(
                juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                juce::Slider::TextEntryBoxPosition::TextBoxBelow);
            mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts,
                "filterMix",
                *mixKnob);
            mixKnob->setScrollWheelEnabled(false);
            mixKnob->setTextValueSuffix("%");
            mixKnob->setTextBoxIsEditable(true);
            addAndMakeVisible(*mixKnob);
            mixLabel = std::make_unique<juce::Label>();
            mixLabel->setText("Mix", juce::NotificationType::dontSendNotification);
            mixLabel->setJustificationType(juce::Justification::centred);
            addAndMakeVisible(*mixLabel);

            frequencyKnob = std::make_unique<juce::Slider>(
                juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                juce::Slider::TextEntryBoxPosition::TextBoxBelow);
            frequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts,
                "filterCutoff",
                *frequencyKnob);
            frequencyKnob->setScrollWheelEnabled(false);
            frequencyKnob->setTextValueSuffix(" Hz");
            frequencyKnob->setTextBoxIsEditable(true);
            addAndMakeVisible(*frequencyKnob);
            frequencyLabel = std::make_unique<juce::Label>();
            frequencyLabel->setText("Cutoff Frequency", juce::NotificationType::dontSendNotification);
            frequencyLabel->setJustificationType(juce::Justification::centred);
            addAndMakeVisible(*frequencyLabel);

            typeSelector = std::make_unique<juce::ComboBox>();
            typeSelector->addItemList(filterTypeChoices, IDX_OFFSET);
            typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                apvts,
                "filterType",
                *typeSelector);
            addAndMakeVisible(*typeSelector);
            typeLabel = std::make_unique<juce::Label>();
            typeLabel->setText("Type", juce::NotificationType::dontSendNotification);
            typeLabel->setJustificationType(juce::Justification::centred);
            addAndMakeVisible(*typeLabel);

            slopeSelector = std::make_unique<juce::ComboBox>();
            slopeSelector->addItemList(filterSlopeChoices, IDX_OFFSET);
            slopeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                apvts,
                "filterSlope",
                *slopeSelector);
            addAndMakeVisible(*slopeSelector);
            slopeLabel = std::make_unique<juce::Label>();
            slopeLabel->setText("Slope", juce::NotificationType::dontSendNotification);
            slopeLabel->setJustificationType(juce::Justification::centred);
            addAndMakeVisible(*slopeLabel);

            nameLabel = std::make_unique<juce::Label>();
            nameLabel->setText("Filter", juce::NotificationType::dontSendNotification);
            nameLabel->setFont(juce::Font(20));
            nameLabel->setJustificationType(juce::Justification::centredLeft);
            addAndMakeVisible(*nameLabel);
        }

        ~FilterEditor() override {}

        void paint(juce::Graphics& g) override {}

        void resized() override
        {
            using TrackInfo = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;
            using Px = juce::Grid::Px;

            juce::Grid grid;
            grid.templateRows = { TrackInfo( Px( LABEL_HEIGHT ) ),
                                TrackInfo( Fr( 1 ) ),
                                TrackInfo( Px( LABEL_HEIGHT ) ),
                                TrackInfo( Fr( 5 ) ),
                                TrackInfo( Px( LABEL_HEIGHT ) ),
                                TrackInfo( Fr( 1 ) ),
                                TrackInfo( Px( LABEL_HEIGHT ) ) };
            grid.templateColumns = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
            grid.items = {
                juce::GridItem( *mixKnob ).withColumn( { 1 } ).withRow( { 2, 7 } ),
                juce::GridItem( *mixLabel ).withColumn( { 1 } ).withRow( { 7 } ),

                juce::GridItem( *typeSelector ).withColumn( { 2 } ).withRow( { 4 } ),
                juce::GridItem( *typeLabel ).withColumn( { 2 } ).withRow( { 5 } ),

                juce::GridItem( *slopeSelector ).withColumn( { 3 } ).withRow( { 4 } ),
                juce::GridItem( *slopeLabel ).withColumn( { 3 } ).withRow( { 5 } ),

                juce::GridItem( *frequencyKnob ).withColumn( { 4 } ).withRow( { 2, 7 } ), 
                juce::GridItem( *frequencyLabel ).withColumn( { 4 } ).withRow( { 7 } ),

                juce::GridItem( *nameLabel ).withColumn( { 1, 5 } ).withRow( { 1 } ) };

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

        std::unique_ptr<juce::Slider> mixKnob, frequencyKnob;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment, frequencyAttachment;

        std::unique_ptr<juce::ComboBox> typeSelector, slopeSelector;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment, slopeAttachment;

        std::unique_ptr<juce::Label> mixLabel, typeLabel, slopeLabel, frequencyLabel, nameLabel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterEditor)
    };
}