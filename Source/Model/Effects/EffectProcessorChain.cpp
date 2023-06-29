/*
==============================================================================

    EffectProcessorChain.cpp
    Created: 26 Mar 2023 8:56:05pm
    Author:  Habama10

==============================================================================
*/

#include <JuceHeader.h>
#include "EffectProcessorChain.h"

namespace Processor::Effects::EffectsChain
{
    EffectProcessorChain::EffectProcessorChain(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
    {
        for(int i = 0; i < FX_MAX_SLOTS ; i++)
        {
            chain.add(std::make_unique<EffectSlot>());
        }

        registerListener(this);
    }

    EffectProcessorChain::~EffectProcessorChain()
    {
        removeListener(this);
    }

    void EffectProcessorChain::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);
        for(auto item : chain)
        {
            item->processor.load()->prepareToPlay(sampleRate, samplesPerBlock);
        }
    }

    void EffectProcessorChain::processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &midiMessages)
    {
        for(auto item : chain)
        {
            if( !item->bypass )
            {
                item->processor.load()->processBlock(buffer, midiMessages);
            }
        }
    }

    void EffectProcessorChain::releaseResources()
    {
        for(auto item : chain)
        {
            item->processor.load()->releaseResources();
        }
    }

    void EffectProcessorChain::registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void EffectProcessorChain::removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    const juce::Array<Editor::Effects::EffectEditor*> EffectProcessorChain::getLoadedEffectEditors() const
    {
        juce::Array<Editor::Effects::EffectEditor*> editorComponents;

        for(auto item : chain)
        {
            editorComponents.add( item->processor.load()->createEditorUnit() );
        }

        return editorComponents;
    }

    void EffectProcessorChain::parameterChanged(const juce::String &parameterID, float newValue)
    {
        if(parameterID.contains("fxBypass"))
        {
            int idx = getFXIndexFromBypassParameterID(parameterID);

            chain[idx]->bypass = bool(newValue);

            if( chain[idx]->bypass )
                chain[idx]->processor.load()->releaseResources();
            else if( getSampleRate() > 0 )
                chain[idx]->processor.load()->prepareToPlay(getSampleRate(), getBlockSize());
        }
        else if(parameterID.contains("fxChoice"))
        {
            jassert(juce::isPositiveAndBelow(newValue, chainChoices.size()));
            int idx = getFXIndexFromChoiceParameterID(parameterID);

            auto choice = static_cast<EffectChoices>(newValue);
            std::unique_ptr<Effects::EffectProcessor> newProcessor;
            switch (choice)
            {
            case Empty:
                newProcessor = std::make_unique<EffectProcessor>();
                break;
            case EQ:
                newProcessor = std::make_unique<Equalizer::EqualizerProcessor>(apvts);
                break;
            case Filter:
                newProcessor = std::make_unique<Filter::FilterProcessor>(apvts);
                break;
            case Compressor:
                newProcessor = std::make_unique<Compressor::CompressorProcessor>(apvts);
                break;
            case Delay:
                newProcessor = std::make_unique<Delay::DelayProcessor>(apvts);
                break;
            case Reverb:
                newProcessor = std::make_unique<Reverb::ReverbProcessor>(apvts);
                break;
            case Chorus:
                newProcessor = std::make_unique<Chorus::ChorusProcessor>(apvts);
                break;
            case Phaser:
                newProcessor = std::make_unique<Phaser::PhaserProcessor>(apvts);
                break;
            case Tremolo:
                newProcessor = std::make_unique<Tremolo::TremoloProcessor>(apvts);
                break;
            default:
                return;
            }

            //If the selected choice is Empty or isn't already on the chain, add it
            if( choice == Empty || !isProcessorInChain(*newProcessor) )
            {
                if( getSampleRate() > 0 )
                {
                    newProcessor->prepareToPlay(getSampleRate(), getBlockSize());
                }
                chain[idx]->processor.store(std::move(newProcessor));
            }
            else    //If the new parameter is already in the chain, discard it and load empty instead
            {
                auto thisParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(parameterID));
                thisParam->setValueNotifyingHost(thisParam->convertTo0to1(Empty));
                chain[idx]->processor.store(std::make_unique<EffectProcessor>());
            }
        }
    }

    bool EffectProcessorChain::isProcessorInChain(const EffectProcessor& processor) const
    {
        for(const auto& item : chain)
        {
            if( typeid( *item->processor.load() ) == typeid( processor ) )
            {
                return true;
            }
        }
        return false;
    }
}