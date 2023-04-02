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

        tuningLabel.setText("Tuning", juce::NotificationType::dontSendNotification);
        tuningLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(tuningLabel);

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

        pitchWheelLabel.setText("PW Range", juce::NotificationType::dontSendNotification);
        pitchWheelLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(pitchWheelLabel);
    }

    ~TuningComponent() override
    {

    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        g.setColour(findColour(juce::GroupComponent::outlineColourId));
        g.drawRect(bounds, 1.f);
    }

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid tuningComponentGrid;
        tuningComponentGrid.templateRows = { TrackInfo( Fr( 1 ) ), TrackInfo( Px( HEIGHT_TUNING_KNOB_PX ) ), TrackInfo( Px( LABEL_HEIGHT ) ), TrackInfo( Fr( 1 ) ) };
        tuningComponentGrid.templateColumns = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
        tuningComponentGrid.items = { 
            juce::GridItem( nullptr ), juce::GridItem( nullptr ), juce::GridItem( nullptr ), juce::GridItem( nullptr ),
            juce::GridItem( *tuningOctavesKnob ), juce::GridItem( *tuningSemitonesKnob ), juce::GridItem( *tuningCentsKnob ), juce::GridItem( *pitchWheelKnob ),
            juce::GridItem( nullptr ), juce::GridItem( tuningLabel ), juce::GridItem( nullptr ), juce::GridItem( pitchWheelLabel ),
            juce::GridItem( nullptr ), juce::GridItem( nullptr ), juce::GridItem( nullptr ), juce::GridItem( nullptr ) };

        tuningComponentGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        tuningComponentGrid.performLayout(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<juce::Slider> tuningOctavesKnob;
    std::unique_ptr<juce::Slider> tuningSemitonesKnob;
    std::unique_ptr<juce::Slider> tuningCentsKnob;
    std::unique_ptr<juce::Slider> pitchWheelKnob;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tuningOctavesKnobAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tuningSemitonesKnobAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tuningCentsKnobAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchWheelKnobAttachment;

    juce::Label tuningLabel;
    juce::Label pitchWheelLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TuningComponent)
};