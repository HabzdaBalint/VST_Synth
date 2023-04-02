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
#include "SynthGainComponent.h"

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

    ~SynthEditor() override
    {

    }

    void paint(juce::Graphics& g) override
    {

    }

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid synthEditorGrid;
        synthEditorGrid.templateRows = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
        synthEditorGrid.templateColumns = { TrackInfo( Fr( 1 ) ), TrackInfo( Fr( 1 ) ) };
        synthEditorGrid.items = { juce::GridItem( *waveformSelector ), juce::GridItem( *unisonComponent ), 
                                  juce::GridItem( *tuningComponent ), juce::GridItem( *adsrComponent ), 
                                  juce::GridItem( *phaseComponent ), juce::GridItem( *synthGainComponent ) };

        synthEditorGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        synthEditorGrid.performLayout(bounds);
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