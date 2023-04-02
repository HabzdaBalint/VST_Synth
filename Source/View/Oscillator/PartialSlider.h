/*
==============================================================================

    PartialSlider.h
    Created: 31 Mar 2023 8:00:34pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

class PartialSlider : public juce::Component
{
public:
    PartialSlider(VST_SynthAudioProcessor& p, int partialIndex) : audioProcessor(p), partialIndex(partialIndex)
    {
        gainSlider = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::LinearBarVertical,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        gainSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            audioProcessor.additiveSynth->getPartialGainParameterName(partialIndex),
            *gainSlider);
        gainSlider->setScrollWheelEnabled(false);
        gainSlider->setTextValueSuffix("%");
        gainSlider->setTextBoxIsEditable(false);
        addAndMakeVisible(*gainSlider);

        phaseSlider = std::make_unique<juce::Slider>(
            juce::Slider::SliderStyle::LinearBar,
            juce::Slider::TextEntryBoxPosition::TextBoxBelow);
        phaseSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts,
            audioProcessor.additiveSynth->getPartialPhaseParameterName(partialIndex),
            *phaseSlider);    
        phaseSlider->setScrollWheelEnabled(false);
        phaseSlider->setTextValueSuffix("%");
        phaseSlider->setTextBoxIsEditable(false);
        addAndMakeVisible(*phaseSlider);

        partialNumber.setText("#" + juce::String(partialIndex + 1), juce::NotificationType::dontSendNotification);
        partialNumber.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(partialNumber);
    }

    ~PartialSlider() override {}

    void paint(juce::Graphics& g) override
    {
        auto bounds = partialNumber.getBounds();
        g.setColour(findColour(juce::GroupComponent::outlineColourId));
        g.drawRect(bounds, 1.f);
    }

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid partialGrid;
        partialGrid.templateRows = { TrackInfo( Fr( 5 ) ), TrackInfo( Px( HEIGHT_PARTIAL_PHASE_PX ) ), TrackInfo( Px( LABEL_HEIGHT ) ) };
        partialGrid.templateColumns = { TrackInfo( Fr( 1 ) ) };
        partialGrid.items = { juce::GridItem( *gainSlider ), juce::GridItem( *phaseSlider ), juce::GridItem( partialNumber ) };

        partialGrid.performLayout(getLocalBounds());
    }
    
private:
    VST_SynthAudioProcessor& audioProcessor;

    int partialIndex;

    std::unique_ptr<juce::Slider> gainSlider;
    std::unique_ptr<juce::Slider> phaseSlider;

    juce::Label partialNumber;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> phaseSliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PartialSlider)
};