/*
==============================================================================

    PhaseComponent.h
    Created: 1 Apr 2023 12:57:17am
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

class PhaseComponent : public juce::Component
{
public:
    PhaseComponent(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        globalPhaseKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        globalPhaseKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "globalPhase",
            *globalPhaseKnob);
        globalPhaseKnob->setScrollWheelEnabled(false);
        globalPhaseKnob->setTextValueSuffix("%");
        globalPhaseKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*globalPhaseKnob);

        globalPhaseLabel.setText("Phase", juce::NotificationType::dontSendNotification);
        globalPhaseLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(globalPhaseLabel);

        randomPhaseKnob = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        randomPhaseKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            "randomPhaseRange",
            *randomPhaseKnob);
        randomPhaseKnob->setScrollWheelEnabled(false);
        randomPhaseKnob->setTextValueSuffix("%");
        randomPhaseKnob->setTextBoxIsEditable(true);
        addAndMakeVisible(*randomPhaseKnob);

        randomPhaseLabel.setText("Phase Randomness", juce::NotificationType::dontSendNotification);
        randomPhaseLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(randomPhaseLabel);
    }

    ~PhaseComponent() override
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

        juce::Grid phaseComponentGrid;
        phaseComponentGrid.templateRows = { TrackInfo( Fr( 1 ) ), TrackInfo( Px( LABEL_HEIGHT ) ) };
        phaseComponentGrid.templateColumns = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
        phaseComponentGrid.items = { 
            juce::GridItem( *globalPhaseKnob ), juce::GridItem( *randomPhaseKnob ),
            juce::GridItem( globalPhaseLabel ), juce::GridItem( randomPhaseLabel ) };

        phaseComponentGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        phaseComponentGrid.performLayout(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<juce::Slider> globalPhaseKnob;
    std::unique_ptr<juce::Slider> randomPhaseKnob;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> globalPhaseKnobAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> randomPhaseKnobAttachment;

    juce::Label globalPhaseLabel;
    juce::Label randomPhaseLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseComponent)
};