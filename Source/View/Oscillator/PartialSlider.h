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

namespace Editor::Oscillator
{
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
                Processor::Synthesizer::OscillatorParameters::getPartialGainParameterID(partialIndex),
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
                Processor::Synthesizer::OscillatorParameters::getPartialPhaseParameterID(partialIndex),
                *phaseSlider);    
            phaseSlider->setScrollWheelEnabled(false);
            phaseSlider->setTextValueSuffix("%");
            phaseSlider->setTextBoxIsEditable(false);
            addAndMakeVisible(*phaseSlider);

            partialNumberLabel = std::make_unique<juce::Label>();
            partialNumberLabel->setText("#" + juce::String(partialIndex + 1), juce::NotificationType::dontSendNotification);
            partialNumberLabel->setJustificationType(juce::Justification::centred);
            addAndMakeVisible(*partialNumberLabel);
        }

        ~PartialSlider() override {}

        void paint(juce::Graphics& g) override
        {
            g.setColour(findColour(juce::GroupComponent::outlineColourId));

            for(auto child : getChildren())
            {
                auto bounds = child->getBounds();
                g.drawRoundedRectangle(bounds.toFloat(), 4.f, OUTLINE_WIDTH);
            }
        }

        void resized() override
        {
            using TrackInfo = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;
            using Px = juce::Grid::Px;

            juce::Grid grid;
            grid.templateRows = { TrackInfo( Fr( 5 ) ), TrackInfo( Px( HEIGHT_PARTIAL_PHASE_PX ) ), TrackInfo( Px( LABEL_HEIGHT + PADDING_PX ) ) };
            grid.templateColumns = { TrackInfo( Fr( 1 ) ) };
            grid.items = {
                juce::GridItem( *gainSlider ).withColumn( { 1 } ).withRow( { 1 } ),
                juce::GridItem( *phaseSlider ).withColumn( { 1 } ).withRow( { 2 } ),
                juce::GridItem( *partialNumberLabel ).withColumn( { 1 } ).withRow( { 3 } ) };

            grid.setGap( Px( PADDING_PX ) );
            auto bounds = getLocalBounds();
            bounds.reduce(PADDING_PX/2, PADDING_PX/2);
            grid.performLayout(bounds);
        }
        
    private:
        VST_SynthAudioProcessor& audioProcessor;

        int partialIndex;

        std::unique_ptr<juce::Slider> gainSlider, phaseSlider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainSliderAttachment, phaseSliderAttachment;

        std::unique_ptr<juce::Label> partialNumberLabel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PartialSlider)
    };
}