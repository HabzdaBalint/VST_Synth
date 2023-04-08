/*
==============================================================================

    SynthEditor.h
    Created: 30 Mar 2023 11:41:33pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "WaveformSelector.h"
#include "TuningComponent.h"
#include "PhaseComponent.h"
#include "UnisonComponent.h"
#include "ADSRComponent.h"
#include "Gain/SynthGainComponent.h"

class SynthEditor : public juce::Component
{
public:
    SynthEditor(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        addAndMakeVisible(*waveformSelector);
        addAndMakeVisible(*tuningComponent);
        addAndMakeVisible(*phaseComponent);
        addAndMakeVisible(*unisonComponent);
        addAndMakeVisible(*adsrComponent);
        addAndMakeVisible(*synthGainComponent);
    }

    ~SynthEditor() override {}

    void paint(juce::Graphics& g) override
    {
        g.setColour(findColour(juce::GroupComponent::outlineColourId));

        for ( auto child : getChildren() )
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
        grid.templateRows = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
        grid.templateColumns = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
        grid.items = {
            juce::GridItem( *waveformSelector ).withColumn( { 1 } ).withRow( { 1 } ),
            juce::GridItem( *unisonComponent ).withColumn( { 2 } ).withRow( { 1 } ), 
            juce::GridItem( *tuningComponent ).withColumn( { 1 } ).withRow( { 2 } ),
            juce::GridItem( *adsrComponent ).withColumn( { 2 } ).withRow( { 2 } ),
            juce::GridItem( *phaseComponent ).withColumn( { 1 } ).withRow( { 3 } ),
            juce::GridItem( *synthGainComponent ).withColumn( { 2 } ).withRow( { 3 } ) };

        grid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        grid.performLayout(bounds);
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    std::unique_ptr<WaveformSelector> waveformSelector = std::make_unique<WaveformSelector>(audioProcessor);
    std::unique_ptr<TuningComponent> tuningComponent = std::make_unique<TuningComponent>(audioProcessor);
    std::unique_ptr<PhaseComponent> phaseComponent = std::make_unique<PhaseComponent>(audioProcessor);
    std::unique_ptr<UnisonComponent> unisonComponent = std::make_unique<UnisonComponent>(audioProcessor);
    std::unique_ptr<SynthGainComponent> synthGainComponent = std::make_unique<SynthGainComponent>(audioProcessor);
    std::unique_ptr<ADSRComponent> adsrComponent = std::make_unique<ADSRComponent>(audioProcessor);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthEditor)
};