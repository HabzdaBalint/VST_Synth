/*
==============================================================================

    EffectEditors.h
    Created: 18 Mar 2023 8:33:31pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"
#include "../EditorParameters.h"

#include "Chorus/ChorusEditor.h"
#include "Compressor/CompressorEditor.h"
#include "Delay/DelayEditor.h"
#include "Equalizer/EqualizerEditor.h"
#include "Filter/FilterEditor.h"
#include "Phaser/PhaserEditor.h"
#include "Reverb/ReverbEditor.h"
#include "Tremolo/TremoloEditor.h"

namespace Editor::Effects
{
    class EffectEditors : public juce::Component,
                          public juce::Timer,
                          public juce::AudioProcessorValueTreeState::Listener
    {
    public:
        EffectEditors(VST_SynthAudioProcessor& p) : audioProcessor(p)
        {
            using namespace Processor::Effects::EffectsChain;

            for(int i = 0; i < FX_MAX_SLOTS; i++)
            {
                audioProcessor.apvts.addParameterListener(getFXChoiceParameterID(i), this);
            }

            startTimerHz(30);
        }

        ~EffectEditors() override
        {
            using namespace Processor::Effects::EffectsChain;

            for(int i = 0; i < FX_MAX_SLOTS; i++)
            {
                audioProcessor.apvts.removeParameterListener(getFXChoiceParameterID(i), this);
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

    private:
        VST_SynthAudioProcessor& audioProcessor;

        juce::OwnedArray<EffectEditor> editors;

        std::atomic<bool> needUpdate { true };

        void timerCallback() override
        {
            if( needUpdate )
            {
                using namespace Processor::Effects::EffectsChain;

                editors.clearQuick(true);
                auto loadedEditors = audioProcessor.fxChain.getLoadedEffectEditors();
                editors.addArray(loadedEditors);

                for(auto& editor : editors)
                {
                    if( editor )
                        addAndMakeVisible(*editor);
                }

                resized();
                needUpdate = false;
            }
        }

        void parameterChanged(const juce::String &parameterID, float newValue) override
        {
            using namespace Processor::Effects::EffectsChain;
            jassert(juce::isPositiveAndBelow(newValue, chainChoices.size()));
            needUpdate = true;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectEditors)
    };
}