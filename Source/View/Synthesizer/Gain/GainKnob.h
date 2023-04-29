/*
==============================================================================

    GainKnob.h
    Created: 31 Mar 2023 8:52:30pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

namespace Editor::Synthesizer
{
    class GainKnob : public juce::Component
    {
    public:
        GainKnob(VST_SynthAudioProcessor& p) : audioProcessor(p)
        {
            gainKnob = std::make_unique<juce::Slider>(
                juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                juce::Slider::TextEntryBoxPosition::TextBoxBelow);
            gainKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.apvts,
                "synthGain",
                *gainKnob);
            gainKnob->setScrollWheelEnabled(false);
            //gainKnob->setTextValueSuffix("%");
            gainKnob->setTextBoxIsEditable(true);
            gainKnob->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, gainKnob->getTextBoxWidth()*1.3, gainKnob->getTextBoxHeight());
            addAndMakeVisible(*gainKnob);

            gainLabel = std::make_unique<juce::Label>();
            gainLabel->setText("Gain", juce::NotificationType::dontSendNotification);
            gainLabel->setJustificationType(juce::Justification::centred);
            addAndMakeVisible(*gainLabel);
        }

        ~GainKnob() override {}

        void paint(juce::Graphics& g) override {}

        void resized() override
        {
            using TrackInfo = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;
            using Px = juce::Grid::Px;

            juce::Grid grid;
            grid.templateRows = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 12 ) ), TrackInfo( Px( LABEL_HEIGHT ) ) , TrackInfo( Fr( 1 ) ) };
            grid.templateColumns = { TrackInfo( Fr( 1 ) ) };
            grid.items = {
                juce::GridItem( *gainKnob ).withColumn( { 1 } ).withRow( { 2, 3 } ),
                juce::GridItem( *gainLabel ).withColumn( { 1 } ).withRow( { 3 } ) };

            grid.setGap( Px( PADDING_PX ) );
            auto bounds = getLocalBounds();
            bounds.reduce(PADDING_PX, PADDING_PX);
            grid.performLayout(bounds);
        }
        
    private:
        VST_SynthAudioProcessor& audioProcessor;

        std::unique_ptr<juce::Slider> gainKnob;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainKnobAttachment;

        std::unique_ptr<juce::Label> gainLabel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainKnob)
    };
}