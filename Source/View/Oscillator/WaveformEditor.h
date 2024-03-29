/*
==============================================================================

    WaveformEditor.h
    Created: 31 Mar 2023 8:00:13pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"
#include "PartialSlider.h"

namespace Editor::Oscillator
{
    class WaveformEditor : public juce::Component
    {
    public:
        WaveformEditor(VST_SynthAudioProcessor& p) : audioProcessor(p)
        {
            for(int i = 0; i < Processor::Synthesizer::HARMONIC_N; i++)
            {
                partialSliders.add(std::make_unique<PartialSlider>(audioProcessor, i));
                addAndMakeVisible(partialSliders[i]);
            }
        }

        ~WaveformEditor() override {}

        void paint(juce::Graphics& g) override {}

        void resized() override
        {
            using TrackInfo = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;
            using Px = juce::Grid::Px;

            juce::Grid grid;
            grid.templateRows = { TrackInfo( Fr( 1 ) ) };

            for (int i = 0; i < Processor::Synthesizer::HARMONIC_N; i++)
            {
                grid.templateColumns.add( TrackInfo( ( Px( WIDTH_PARTIAL_SLIDERS_PX ) ) ) );
                grid.items.add( juce::GridItem( partialSliders[i] ).withColumn( { i + 1 } ).withRow( { 1 } ) );
            }

            auto bounds = getLocalBounds(); //viewport width
            bounds.setWidth(Processor::Synthesizer::HARMONIC_N * ( WIDTH_PARTIAL_SLIDERS_PX + PADDING_PX/2 ) + PADDING_PX/2 );
            setBounds(bounds);

            grid.setGap( Px( PADDING_PX/2 ) );    //grid
            bounds = getLocalBounds();
            bounds.reduce(PADDING_PX/2, PADDING_PX/2);
            grid.performLayout(bounds);
        }
        
    private:
        VST_SynthAudioProcessor& audioProcessor;

        juce::OwnedArray<PartialSlider> partialSliders;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformEditor)
    };
}