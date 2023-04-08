/*
==============================================================================

    ADSRComponent.h
    Created: 31 Mar 2023 11:15:45pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

class ADSRComponent : public juce::Component
{
public:
    ADSRComponent(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        attackSlider = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::LinearVertical,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        attackSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "amplitudeADSRAttack",
            *attackSlider);
        attackSlider->setScrollWheelEnabled(false);
        attackSlider->setTextValueSuffix(" ms");
        attackSlider->setTextBoxIsEditable(true);
        addAndMakeVisible(*attackSlider);

        attackLabel = std::make_unique<juce::Label>();
        attackLabel->setText("Attack", juce::NotificationType::dontSendNotification);
        attackLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*attackLabel);

        decaySlider = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::LinearVertical,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        decaySliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "amplitudeADSRDecay",
            *decaySlider);
        decaySlider->setScrollWheelEnabled(false);
        decaySlider->setTextValueSuffix(" ms");
        decaySlider->setTextBoxIsEditable(true);
        addAndMakeVisible(*decaySlider);

        decayLabel = std::make_unique<juce::Label>();
        decayLabel->setText("Decay", juce::NotificationType::dontSendNotification);
        decayLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*decayLabel);

        sustainSlider = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::LinearVertical,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        sustainSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "amplitudeADSRSustain",
            *sustainSlider);
        sustainSlider->setScrollWheelEnabled(false);
        sustainSlider->setTextValueSuffix("%");
        sustainSlider->setTextBoxIsEditable(true);
        addAndMakeVisible(*sustainSlider);

        sustainLabel = std::make_unique<juce::Label>();
        sustainLabel->setText("Sustain", juce::NotificationType::dontSendNotification);
        sustainLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*sustainLabel);

        releaseSlider = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::LinearVertical,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        releaseSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "amplitudeADSRRelease",
            *releaseSlider);
        releaseSlider->setScrollWheelEnabled(false);
        releaseSlider->setTextValueSuffix(" ms");
        releaseSlider->setTextBoxIsEditable(true);
        addAndMakeVisible(*releaseSlider);

        releaseLabel = std::make_unique<juce::Label>();
        releaseLabel->setText("Release", juce::NotificationType::dontSendNotification);
        releaseLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*releaseLabel);
    }

    ~ADSRComponent() override {}

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid adsrGrid;
        adsrGrid.templateRows = { TrackInfo( Fr( 1 ) ), TrackInfo( Px( LABEL_HEIGHT ) ) };
        adsrGrid.templateColumns = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
        adsrGrid.items = { 
            juce::GridItem( *attackSlider ).withColumn({1}).withRow({1}),
            juce::GridItem( *attackLabel ).withColumn({1}).withRow({2}),

            juce::GridItem( *decaySlider ).withColumn({2}).withRow({1}),
            juce::GridItem( *decayLabel ).withColumn({2}).withRow({2}),

            juce::GridItem( *sustainSlider ).withColumn({3}).withRow({1}),
            juce::GridItem( *sustainLabel ).withColumn({3}).withRow({2}),

            juce::GridItem( *releaseSlider ).withColumn({4}).withRow({1}),
            juce::GridItem( *releaseLabel ).withColumn({4}).withRow({2}) };

        adsrGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        adsrGrid.performLayout(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<juce::Slider> attackSlider, decaySlider, sustainSlider, releaseSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackSliderAttachment, decaySliderAttachment, sustainSliderAttachment, releaseSliderAttachment;

    std::unique_ptr<juce::Label> attackLabel, decayLabel, sustainLabel, releaseLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ADSRComponent)
};