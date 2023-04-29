/*
==============================================================================

    EqualizerEditor.h
    Created: 6 Apr 2023 2:02:53pm
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
    class EqualizerEditor : public EffectEditor
    {
    public:
        EqualizerEditor(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
        {
            using namespace Processor::Effects::Equalizer;

            for (size_t i = 0; i < 10; i++)
            {
                bandSliders.add(std::make_unique<juce::Slider>(
                    juce::Slider::SliderStyle::LinearVertical,
                    juce::Slider::TextEntryBoxPosition::TextBoxBelow));
                bandSliders[i]->setScrollWheelEnabled(false);
                bandSliders[i]->setTextValueSuffix(" dB");
                bandSliders[i]->setTextBoxIsEditable(true);
                addAndMakeVisible(*bandSliders[i]);

                bandSliderAttachments.add(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                    apvts,
                    getBandGainParameterID(i),
                    *bandSliders[i]));
                bandLabels.add(std::make_unique<juce::Label>());

                bandLabels[i]->setText(getBandFrequencyLabel(i), juce::NotificationType::dontSendNotification);
                bandLabels[i]->setJustificationType(juce::Justification::centred);
                addAndMakeVisible(*bandLabels[i]);

                nameLabel = std::make_unique<juce::Label>();
                nameLabel->setText("EQ", juce::NotificationType::dontSendNotification);
                nameLabel->setFont(juce::Font(20));
                nameLabel->setJustificationType(juce::Justification::centredLeft);
                addAndMakeVisible(*nameLabel);
            }
        }

        ~EqualizerEditor() override {}

        void paint(juce::Graphics& g) override {}

        void resized() override
        {
            using TrackInfo = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;
            using Px = juce::Grid::Px;

            juce::Grid grid;
            grid.templateRows = { TrackInfo( Px( LABEL_HEIGHT ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Px( LABEL_HEIGHT ) ) };

            grid.items.add( juce::GridItem( *nameLabel ).withColumn( { 1, 11 } ).withRow( { 1 } ) );

            for (int i = 0; i < Processor::Effects::Equalizer::NUM_BANDS; i++)
            {
                grid.templateColumns.add( TrackInfo( Fr( 1 ) ) );
                grid.items.add( juce::GridItem(*bandSliders[i]).withColumn( { i + 1 } ).withRow( { 2 } ) );
                grid.items.add( juce::GridItem( *bandLabels[i] ).withColumn( { i + 1 } ).withRow( { 3 } ) );
            }

            grid.rowGap = Px( PADDING_PX );
            auto bounds = getLocalBounds();
            bounds.reduce(PADDING_PX, PADDING_PX);
            grid.performLayout(bounds);
        }

        const int getIdealHeight() override
        {
            return 225 + 2 * PADDING_PX;
        }


    private:
        juce::AudioProcessorValueTreeState& apvts;

        juce::OwnedArray<juce::Slider> bandSliders;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> bandSliderAttachments;
        juce::OwnedArray<juce::Label> bandLabels;

        std::unique_ptr<juce::Label> nameLabel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqualizerEditor)
    };
}