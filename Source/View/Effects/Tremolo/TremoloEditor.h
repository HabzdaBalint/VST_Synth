/*
==============================================================================

    TremoloEditor.h
    Created: 6 Apr 2023 3:02:00pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "../EffectEditorUnit.h"

class TremoloEditor : public EffectEditorUnit
{
public:
    TremoloEditor(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        mixKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "tremoloMix",
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
            audioProcessor.apvts,
            "tremoloRate",
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
            audioProcessor.apvts,
            "tremoloDepth",
            *depthKnob);
        depthKnob->setScrollWheelEnabled(false);
        depthKnob->setTextValueSuffix("%");
        depthKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*depthKnob);
        depthLabel = std::make_unique<juce::Label>();
        depthLabel->setText("Depth", juce::NotificationType::dontSendNotification);
        depthLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*depthLabel);

        autoPanToggle = std::make_unique<juce::ToggleButton>( "Auto-Pan" );
        autoPanAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            p.apvts,
            "tremoloAutoPan",
            *autoPanToggle);
        addAndMakeVisible(*autoPanToggle);

        nameLabel = std::make_unique<juce::Label>();
        nameLabel->setText("Tremolo", juce::NotificationType::dontSendNotification);
        nameLabel->setFont(juce::Font(20));
        nameLabel->setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(*nameLabel);
    }

    ~TremoloEditor() override {}

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid grid;
        grid.templateRows = { TrackInfo( Px( LABEL_HEIGHT ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Px( LABEL_HEIGHT ) ) };
        grid.templateColumns = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
        grid.items = { 
            juce::GridItem( *mixKnob ).withColumn( { 1 } ).withRow( { 2 } ),
            juce::GridItem( *mixLabel ).withColumn( { 1 } ).withRow( { 3 } ),

            juce::GridItem( *rateKnob ).withColumn( { 2 } ).withRow( { 2 } ),
            juce::GridItem( *rateLabel ).withColumn( { 2 } ).withRow( { 3 } ),
            
            juce::GridItem( *depthKnob ).withColumn( { 3 } ).withRow( { 2 } ),
            juce::GridItem( *depthLabel ).withColumn( { 3 } ).withRow( { 3 } ),

            juce::GridItem( *autoPanToggle ).withColumn( { 4 } ).withRow( { 2 } ),
            
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
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<juce::Slider> mixKnob, rateKnob, depthKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment, rateAttachment, depthAttachment;
    std::unique_ptr<juce::Label> mixLabel, rateLabel, depthLabel, nameLabel;

    std::unique_ptr<juce::ToggleButton> autoPanToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoPanAttachment;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TremoloEditor)
};