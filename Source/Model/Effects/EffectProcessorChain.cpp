/*
==============================================================================

    EffectProcessorChain.cpp
    Created: 26 Mar 2023 8:56:05pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "EffectProcessorChain.h"

namespace Processor::Effects::EffectsChain
{
    EffectProcessorChain::EffectProcessorChain(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
    {
        for(int i = 0; i < FX_MAX_SLOTS ; i++)
        {
            //chain.add(std::make_unique<EffectSlot>());
            chain.add(std::make_unique<Utils::TripleBuffer<EffectSlot>>());
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
            item->acquire();
            auto& localSlot = item->read();

            if( localSlot.processor != nullptr )
                localSlot.processor->prepareToPlay(sampleRate, samplesPerBlock);
        }
    }

    void EffectProcessorChain::releaseResources()
    {
        for(auto item : chain)
        {
            item->acquire();
            auto& localSlot = item->read();
            if( localSlot.processor != nullptr )
                localSlot.processor->releaseResources();
        }
    }

    void EffectProcessorChain::processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &midiMessages)
    {
        for(auto item : chain)
        {
            item->acquire();
            auto& localSlot = item->read();
            if( !localSlot.bypass )
            {
                if( localSlot.processor != nullptr )
                    localSlot.processor->processBlock(buffer, midiMessages);
            }
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
            item->acquire();
            auto& localSlot = item->read();

            if( localSlot.processor )
                editorComponents.add( localSlot.processor->createEditorUnit() );
            else
                editorComponents.add( nullptr );
        }

        return editorComponents;
    }

    void EffectProcessorChain::parameterChanged(const juce::String &parameterID, float newValue)
    {
        if(parameterID.contains("fxBypass"))
        {
            int idx = getFXIndexFromBypassParameterID(parameterID);

            auto& localSlot = chain[idx]->write();
            localSlot.bypass = (bool)newValue;

            if( localSlot.processor )
            {
                if( localSlot.bypass )
                    localSlot.processor->releaseResources();
                else if( getSampleRate() > 0 )
                    localSlot.processor->prepareToPlay(getSampleRate(), getBlockSize());
            }

            chain[idx]->release();
        }
        else if(parameterID.contains("fxChoice"))
        {
            jassert(juce::isPositiveAndBelow(newValue, chainChoices.size()));

            auto choice = static_cast<EffectChoices>(newValue);
            int idx = getFXIndexFromChoiceParameterID(parameterID);

            std::unique_ptr<Effects::EffectProcessor> newProcessor;
            switch (choice)
            {
            case Empty:
                newProcessor = nullptr;
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
                newProcessor = nullptr;
                break;
            }

            //If the selected effect is Empty or isn't already on the chain, add it
            if( !newProcessor || !isProcessorInChain(typeid(*newProcessor)) )
            {
                if(newProcessor && getSampleRate() > 0)
                {
                    newProcessor->prepareToPlay(getSampleRate(), getBlockSize());
                }

                auto& localSlot = chain[idx]->write();

                if( localSlot.processor )
                {
                    localSlot.processor->releaseResources();
                }
                localSlot.processor = std::move(newProcessor);

                chain[idx]->release();
            }
            else    //If the new parameter is already in the chain, discard it and load empty instead
            {
                auto thisParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(parameterID));
                thisParam->setValueNotifyingHost(thisParam->convertTo0to1(Empty));

                auto& localSlot = chain[idx]->write();

                localSlot.processor = nullptr;

                chain[idx]->release();
            }
        }
    }

    bool EffectProcessorChain::isProcessorInChain(const std::type_info& type) const
    {
        for(const auto& item : chain)
        {
            item->acquire();
            auto& localSlot = item->read();

            if( localSlot.processor && typeid( *localSlot.processor ) == type)
            {
                return true;
            }
        }
        return false;
    }

}