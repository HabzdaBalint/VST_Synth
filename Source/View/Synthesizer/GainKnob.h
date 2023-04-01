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
        gainKnob->setTextValueSuffix(" dB");
        gainKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*gainKnob);

        gainLabel.setText("Gain", juce::NotificationType::dontSendNotification);
        gainLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(gainLabel);
    }

    ~GainKnob() override {}

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid gainKnobGrid;
        gainKnobGrid.templateRows = { TrackInfo( Fr( 1 ) ), TrackInfo( Px( HEIGHT_GAIN_KNOB_PX ) ), TrackInfo( Px( 20 ) ), TrackInfo( Fr( 1 ) ) };
        gainKnobGrid.templateColumns = { TrackInfo( Fr( 1 ) ) };
        gainKnobGrid.items = { juce::GridItem( nullptr ), juce::GridItem( *gainKnob ), juce::GridItem( gainLabel ), juce::GridItem( nullptr ) };

        gainKnobGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        gainKnobGrid.performLayout(bounds);
    }
    
private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<juce::Slider> gainKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainKnobAttachment;

    juce::Label gainLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainKnob)
};