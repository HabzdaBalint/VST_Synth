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

#include "EffectEditorUnit.h"

#include "Equalizer/EqualizerEditor.h"
#include "Filter/FilterEditor.h"
#include "Compressor/CompressorEditor.h"
#include "Delay/DelayEditor.h"
#include "Reverb/ReverbEditor.h"
#include "Chorus/ChorusEditor.h"
#include "Phaser/PhaserEditor.h"
#include "Tremolo/TremoloEditor.h"

class FXChainEditor : public juce::Component
{
public:
    FXChainEditor(VST_SynthAudioProcessor& p, juce::Array<int>& loadedFx) : audioProcessor(p), loadedFx(loadedFx)
    {
        editors.add(nullptr);
        editors.add(std::make_unique<EqualizerEditor>(p));
        editors.add(std::make_unique<FilterEditor>(p));
        editors.add(std::make_unique<CompressorEditor>(p));
        editors.add(std::make_unique<DelayEditor>(p));
        editors.add(std::make_unique<ReverbEditor>(p));
        editors.add(std::make_unique<ChorusEditor>(p));
        editors.add(std::make_unique<PhaserEditor>(p));
        editors.add(std::make_unique<TremoloEditor>(p));

        for (auto editor : editors)
        {
            if(editor != nullptr)
                addAndMakeVisible(editor);
        }
    }

    ~FXChainEditor() override {}

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

        for (auto child : getChildren())
        {
            auto bounds = child->getBounds();
            bounds.setHeight(0);
            child->setBounds(bounds);
        }

        juce::Grid effectsEditorsGrid;
        effectsEditorsGrid.templateColumns = { TrackInfo( Fr( 1 ) ) };

        int height = 0, counter = 0;
        for (auto idx : loadedFx)
        {
            if(idx > 0 && idx < editors.size())
            {
                effectsEditorsGrid.items.add( juce::GridItem( *editors[idx] ) );
                effectsEditorsGrid.templateRows.add ( TrackInfo( Px( editors[idx]->getIdealHeight() ) ) );
                height += editors[idx]->getIdealHeight();
                counter++;
            }
        }

        //for the viewport    
        auto bounds = getLocalBounds();
        bounds.setHeight(height + ( counter + 1 ) * PADDING_PX );
        setBounds(bounds);

        //gird
        effectsEditorsGrid.setGap( Px( PADDING_PX ) );
        bounds = getLocalBounds();
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

    juce::OwnedArray<EffectEditorUnit> editors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXChainEditor)
};