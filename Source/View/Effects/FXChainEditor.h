/*
==============================================================================

    FXChainEditor.h
    Created: 18 Mar 2023 8:33:31pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "Chorus/ChorusEditor.h"

class FXChainEditor : public juce::Component
{
public:
    FXChainEditor(VST_SynthAudioProcessor& p, juce::Array<int>& loadedFx) : audioProcessor(p), loadedFx(loadedFx)
    {   //todo: construct items in order, also set desired height during construction
        editors.add(std::make_unique<ChorusEditor>(p));
        editors.add(std::make_unique<ChorusEditor>(p));
        editors.add(std::make_unique<ChorusEditor>(p));
        editors.add(std::make_unique<ChorusEditor>(p));
        editors.add(std::make_unique<ChorusEditor>(p));
        editors.add(std::make_unique<ChorusEditor>(p));
        editors.add(std::make_unique<ChorusEditor>(p));
        editors.add(std::make_unique<ChorusEditor>(p));

        for (auto editor : editors)
        {
            addAndMakeVisible(editor);
        }
    }

    ~FXChainEditor() override {}

    void paint(juce::Graphics& g) override {}

    void resized() override
    {
        using TrackInfo = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;
        using Px = juce::Grid::Px;

        juce::Grid effectsEditorsGrid;
        effectsEditorsGrid.templateColumns = { TrackInfo( Fr( 1 ) ) };

        for (auto idx : loadedFx)
        {
            idx -= 1; //first element is ,,Empty"
            if(idx >= 0)
            {
                effectsEditorsGrid.items.add( juce::GridItem( *editors[idx] ) );    //todo: implement a custom component for fx units that return a number here
                effectsEditorsGrid.templateRows.add ( TrackInfo( Px( editors[idx]->getIdealHeight() ) ) );
            }
        }       

        effectsEditorsGrid.setGap( Px( PADDING_PX ) );
        auto bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        effectsEditorsGrid.performLayout(bounds);
    }

    void updateChainEditor()
    {
        resized();
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    const juce::Array<int>& loadedFx;

    juce::OwnedArray<juce::Component> editors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXChainEditor)
};