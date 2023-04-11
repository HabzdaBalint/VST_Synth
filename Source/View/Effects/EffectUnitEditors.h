/*
==============================================================================

    EffectUnitEditors.h
    Created: 18 Mar 2023 8:33:31pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "EffectEditors/Chorus/ChorusEditor.h"
#include "EffectEditors/Compressor/CompressorEditor.h"
#include "EffectEditors/Delay/DelayEditor.h"
#include "EffectEditors/Equalizer/EqualizerEditor.h"
#include "EffectEditors/Filter/FilterEditor.h"
#include "EffectEditors/Phaser/PhaserEditor.h"
#include "EffectEditors/Reverb/ReverbEditor.h"
#include "EffectEditors/Tremolo/TremoloEditor.h"

class EffectUnitEditors : public juce::Component,
                          public juce::AudioProcessorValueTreeState::Listener
{
public:
    EffectUnitEditors(VST_SynthAudioProcessor& p) : audioProcessor(p)
    {
        for (size_t i = 0; i < Effects::EffectsChain::FX_MAX_SLOTS; i++)
        {
            audioProcessor.apvts.addParameterListener(Effects::EffectsChain::FXProcessorChain::getFXChoiceParameterName(i), this);
        }

        auto loadedEditors = audioProcessor.fxChain->getLoadedEffectEditors();
        editors.addArray(loadedEditors);

        for (auto editor : editors)
        {
            if (editor)
                addAndMakeVisible(*editor);
        }
    }

    ~EffectUnitEditors() override
    {
        for (size_t i = 0; i < Effects::EffectsChain::FX_MAX_SLOTS; i++)
        {
            audioProcessor.apvts.removeParameterListener(Effects::EffectsChain::FXProcessorChain::getFXChoiceParameterName(i), this);
        }
    }

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

        juce::Grid grid;
        grid.templateColumns = { TrackInfo( Fr( 1 ) ) };

        int height = 0, counter = 0;

        for (auto editor : editors)
        {
            if(editor)
            {
                grid.items.add( juce::GridItem( *editor ) );
                auto editorHeight = editor->getIdealHeight();
                grid.templateRows.add ( TrackInfo( Px( editorHeight ) ) );
                height += editorHeight;
                counter++;
            }
        }

        //for the viewport    
        auto bounds = getLocalBounds();
        bounds.setHeight(height + ( counter + 1 ) * PADDING_PX );
        setBounds(bounds);

        //gird
        grid.setGap( Px( PADDING_PX ) );
        bounds = getLocalBounds();
        bounds.reduce(PADDING_PX, PADDING_PX);
        grid.performLayout(bounds);
    }

    void parameterChanged(const juce::String &parameterID, float newValue) override
    {
        //Querying fxProcessor via getLoadedEffectEditors doesn't work because parameterChanged calls might happen in the wrong order, where this component gets a list of components that doesn't yet include the newly selected fx unit's editor
        jassert(juce::isPositiveAndBelow(newValue, Effects::EffectsChain::choices.size()));

        int idx = std::stoi(parameterID.trimCharactersAtStart("fxChoice").toStdString());

        std::unique_ptr<EffectEditorUnit> newEditor;
        switch ((int)newValue)
        {
        case 0:
            newEditor = nullptr;
            break;
        case 1:
            newEditor = std::make_unique<EqualizerEditor>(audioProcessor.apvts);
            break;
        case 2:
            newEditor = std::make_unique<FilterEditor>(audioProcessor.apvts);
            break;
        case 3:
            newEditor = std::make_unique<CompressorEditor>(audioProcessor.apvts);
            break;
        case 4:
            newEditor = std::make_unique<DelayEditor>(audioProcessor.apvts);
            break;
        case 5:
            newEditor = std::make_unique<ReverbEditor>(audioProcessor.apvts);
            break;
        case 6:
            newEditor = std::make_unique<ChorusEditor>(audioProcessor.apvts);
            break;
        case 7:
            newEditor = std::make_unique<PhaserEditor>(audioProcessor.apvts);
            break;
        case 8:
            newEditor = std::make_unique<TremoloEditor>(audioProcessor.apvts);
            break;
        default:
            newEditor = nullptr;
            break;
        }

        editors.remove(idx);
        editors.insert(idx, std::move(newEditor));

        if(editors[idx])
            addAndMakeVisible(*editors[idx]);

        resized();
    }

private:
    VST_SynthAudioProcessor& audioProcessor;

    juce::OwnedArray<EffectEditorUnit> editors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectUnitEditors)
};