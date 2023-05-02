/*
==============================================================================

    SynthGain.h
    Created: 31 Mar 2023 8:26:31pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "GainKnob.h"
#include "LevelMeter.h"

namespace Editor::Synthesizer
{
    class SynthGainComponent : public juce::Component
    {
    public:
        SynthGainComponent(VST_SynthAudioProcessor& p) : audioProcessor(p)
        {
            addAndMakeVisible(*gainKnob);

            auto numChannels = p.getTotalNumOutputChannels();
            for(int i = 0; i < numChannels; i++)
            {
                levelMeters.add(std::make_unique<LevelMeter>(p, i));
                addAndMakeVisible(*levelMeters[i]);
            }
        }

        ~SynthGainComponent() override {}

        void paint(juce::Graphics& g) override {}

        void resized() override
        {
            using TrackInfo = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;
            using Px = juce::Grid::Px;
            
            juce::Grid grid;
            grid.templateColumns = { TrackInfo( Fr( 2 ) ), TrackInfo( Fr( 5 ) ) };

            for(int i = 0; i < levelMeters.size(); i++)
            {
                grid.templateRows.add( TrackInfo( Fr( 1 ) ) );
                grid.items.add( juce::GridItem( *levelMeters[i] ).withColumn( { 2 } ).withRow( { (i+1) } ) );
            }
            grid.items.add( juce::GridItem( *gainKnob ).withColumn( { 1 } ).withRow( { 1, levelMeters.size() + 1 } ) );

            grid.setGap( Px( PADDING_PX ) );
            auto bounds = getLocalBounds();
            bounds.reduce(PADDING_PX, PADDING_PX);
            grid.performLayout(bounds);
        }

    private:
        VST_SynthAudioProcessor& audioProcessor;

        std::unique_ptr<GainKnob> gainKnob = std::make_unique<GainKnob>(audioProcessor);
        juce::OwnedArray<LevelMeter> levelMeters;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthGainComponent)
    };
}