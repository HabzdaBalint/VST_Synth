/*
==============================================================================

    ChorusEditor.h
    Created: 5 Apr 2023 4:38:09pm
    Author:  habzd

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

class ChorusEditor : public juce::Component
{
public:
    ChorusEditor(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        mixKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "chorusMix",
            *mixKnob);
        mixKnob->setScrollWheelEnabled(true);
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
            "chorusRate",
            *rateKnob);
        rateKnob->setScrollWheelEnabled(true);
        rateKnob->setTextValueSuffix("Hz");
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
            audioProcessor.apvts,
            "chorusdelay",
            *delayKnob);
        delayKnob->setScrollWheelEnabled(true);
        delayKnob->setTextValueSuffix("%");
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
            audioProcessor.apvts,
            "chorusDepth",
            *depthKnob);
        depthKnob->setScrollWheelEnabled(true);
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
            audioProcessor.apvts,
            "chorusfeedback",
            *feedbackKnob);
        feedbackKnob->setScrollWheelEnabled(true);
        feedbackKnob->setTextValueSuffix("%");
        feedbackKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*feedbackKnob);
        feedbackLabel = std::make_unique<juce::Label>();
        feedbackLabel->setText("Feedback", juce::NotificationType::dontSendNotification);
        feedbackLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*feedbackLabel);
    }

    ~ChorusEditor() override {}

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        g.setColour(findColour(juce::GroupComponent::outlineColourId));
        g.drawRect(bounds, 1.f);

        bounds = mixKnob->getLocalBounds();
        bounds.enlargeIfAdjacent(mixLabel->getLocalBounds());
        g.drawRect(bounds, 1.f);
    }

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid chorusEditorGrid;
        chorusEditorGrid.templateRows = { TrackInfo( Px( HEIGHT_CHORUS_EDITOR_PX ) ), TrackInfo( Px( LABEL_HEIGHT ) ) };
        chorusEditorGrid.templateColumns = { TrackInfo( Px( WIDTH_MIX_KNOB ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
        chorusEditorGrid.items = { 
            juce::GridItem( *mixKnob ), juce::GridItem( *rateKnob ), juce::GridItem( *delayKnob ), juce::GridItem( *depthKnob ), juce::GridItem( *feedbackKnob ),
            juce::GridItem( *mixLabel ), juce::GridItem( *rateLabel ), juce::GridItem( *delayLabel ), juce::GridItem( *depthLabel ), juce::GridItem( *feedbackLabel ) };

        chorusEditorGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        chorusEditorGrid.performLayout(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<juce::Slider> mixKnob, rateKnob, delayKnob, depthKnob, feedbackKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment, rateAttachment, delayAttachment, depthAttachment, feedbackAttachment;
    std::unique_ptr<juce::Label> mixLabel, rateLabel, delayLabel, depthLabel, feedbackLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChorusEditor)
};