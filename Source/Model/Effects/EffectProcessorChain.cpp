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

namespace Effects::EffectsChain
{
    EffectProcessorChain::EffectProcessorChain(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
    {
        for (size_t i = 0; i < FX_MAX_SLOTS ; i++)
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
            std::shared_ptr<Effects::EffectProcessor> localProcessor = item->processor;
            if( localProcessor != nullptr )
                localProcessor->prepareToPlay(sampleRate, samplesPerBlock);
        }
    }

    void EffectProcessorChain::releaseResources()
    {
        for(auto item : chain)
        {
            std::shared_ptr<Effects::EffectProcessor> localProcessor = item->processor;
            if( localProcessor != nullptr )
                localProcessor->releaseResources();
        }
    }

    void EffectProcessorChain::processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &midiMessages)
    {
        for(auto item : chain)
        {
            if( !item->bypass )
            {
                std::shared_ptr<Effects::EffectProcessor> localProcessor = item->processor;
                if( localProcessor != nullptr )
                    localProcessor->processBlock(buffer, midiMessages);
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

    const juce::Array<EffectEditor*> EffectProcessorChain::getLoadedEffectEditors() const
    {
        juce::Array<EffectEditor*> editorComponents;

        for(auto item : chain)
        {
            if( item->processor.load() )
                editorComponents.add( item->processor.load()->createEditorUnit() );
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
            chain[idx]->bypass = (bool)newValue;

            if(chain[idx]->processor.load())
            {
                if( chain[idx]->bypass )
                    chain[idx]->processor.load()->releaseResources();
                else if( getSampleRate() > 0 )
                    chain[idx]->processor.load()->prepareToPlay(getSampleRate(), getBlockSize());
            }
        }
        else if(parameterID.contains("fxChoice"))
        {
            jassert(juce::isPositiveAndBelow(newValue, choices.size()));

            auto choice = static_cast<EffectChoices>(newValue);
            int idx = getFXIndexFromChoiceParameterID(parameterID);

            std::unique_ptr<Effects::EffectProcessor> newProcessor;
            switch (choice)
            {
            case Empty:
                newProcessor = nullptr;
                break;
            case EQ:
                newProcessor = std::make_unique<Effects::Equalizer::EqualizerProcessor>(apvts);
                break;
            case Filter:
                newProcessor = std::make_unique<Effects::Filter::FilterProcessor>(apvts);
                break;
            case Compressor:
                newProcessor = std::make_unique<Effects::Compressor::CompressorProcessor>(apvts);
                break;
            case Delay:
                newProcessor = std::make_unique<Effects::Delay::DelayProcessor>(apvts);
                break;
            case Reverb:
                newProcessor = std::make_unique<Effects::Reverb::ReverbProcessor>(apvts);
                break;
            case Chorus:
                newProcessor = std::make_unique<Effects::Chorus::ChorusProcessor>(apvts);
                break;
            case Phaser:
                newProcessor = std::make_unique<Effects::Phaser::PhaserProcessor>(apvts);
                break;
            case Tremolo:
                newProcessor = std::make_unique<Effects::Tremolo::TremoloProcessor>(apvts);
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

                if(chain[idx]->processor.load())
                {
                    chain[idx]->processor.load()->releaseResources();
                }
                chain[idx]->processor = std::move(newProcessor);
            }
            else    //If the new parameter is already in the chain, discard it and load empty instead
            {
                auto thisParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(parameterID));
                thisParam->setValueNotifyingHost(thisParam->convertTo0to1(Empty));
                chain[idx]->processor.store(nullptr);
            }
        }
    }

    bool EffectProcessorChain::isProcessorInChain(const std::type_info& type) const
    {
        for(const auto& item : chain)
        {
            if(item->processor.load() && typeid(*item->processor.load()) == type)
            {
                return true;
            }
        }
        return false;
    }

}