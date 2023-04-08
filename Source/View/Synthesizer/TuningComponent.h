/*
==============================================================================

    TuningComponent.h
    Created: 1 Apr 2023 1:14:52am
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

class TuningComponent : public juce::Component
{
public:
    TuningComponent(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        tuningOctavesKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        tuningOctavesKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "oscillatorOctaves",
            *tuningOctavesKnob);
        tuningOctavesKnob->setScrollWheelEnabled(false);
        tuningOctavesKnob->setTextValueSuffix(" Octaves");
        tuningOctavesKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*tuningOctavesKnob);

        tuningSemitonesKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        tuningSemitonesKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "oscillatorSemitones",
            *tuningSemitonesKnob);
        tuningSemitonesKnob->setScrollWheelEnabled(false);
        tuningSemitonesKnob->setTextValueSuffix(" Semitones");
        tuningSemitonesKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*tuningSemitonesKnob);

        tuningCentsKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        tuningCentsKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "oscillatorFine",
            *tuningCentsKnob);
        tuningCentsKnob->setScrollWheelEnabled(false);
        tuningCentsKnob->setTextValueSuffix(" Cents");
        tuningCentsKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*tuningCentsKnob);

        tuningLabel = std::make_unique<juce::Label>();
        tuningLabel->setText("Tuning", juce::NotificationType::dontSendNotification);
        tuningLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*tuningLabel);

        pitchWheelKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        pitchWheelKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "pitchWheelRange",
            *pitchWheelKnob);
        pitchWheelKnob->setScrollWheelEnabled(false);
        pitchWheelKnob->setTextValueSuffix(" Semitones");
        pitchWheelKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*pitchWheelKnob);

        pitchWheelLabel = std::make_unique<juce::Label>();
        pitchWheelLabel->setText("PW Range", juce::NotificationType::dontSendNotification);
        pitchWheelLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*pitchWheelLabel);
    }

    ~TuningComponent() override {}

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid grid;
        grid.templateRows = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 8 ) ), TrackInfo( Px( LABEL_HEIGHT ) ), TrackInfo( Fr( 1 ) ) };
        grid.templateColumns = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
        grid.items = { 
            juce::GridItem( *tuningOctavesKnob ).withColumn( { 1 } ).withRow( { 2, 3 } ),
            juce::GridItem( *tuningSemitonesKnob ).withColumn( { 2 } ).withRow( { 2, 3 } ),
            juce::GridItem( *tuningCentsKnob ).withColumn( { 3 } ).withRow( { 2, 3 } ),
            juce::GridItem( *tuningLabel ).withColumn( { 1, 4 } ).withRow( { 3 } ),

            juce::GridItem( *pitchWheelKnob ).withColumn( { 4 } ).withRow( { 2, 3 } ),
            juce::GridItem( *pitchWheelLabel ).withColumn( { 4 } ).withRow( { 3 } ) };

        grid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        grid.performLayout(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<juce::Slider> tuningOctavesKnob, tuningSemitonesKnob, tuningCentsKnob, pitchWheelKnob;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tuningOctavesKnobAttachment, tuningSemitonesKnobAttachment, tuningCentsKnobAttachment, pitchWheelKnobAttachment;

    std::unique_ptr<juce::Label> tuningLabel, pitchWheelLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TuningComponent)
};