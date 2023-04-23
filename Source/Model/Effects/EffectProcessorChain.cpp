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
        else
        {
            jassert(juce::isPositiveAndBelow(newValue, choices.size()));

            int idx = getFXIndexFromChoiceParameterID(parameterID);

            std::unique_ptr<Effects::EffectProcessor> newProcessor;
            switch ((int)newValue)
            {
            case 0:
                newProcessor = nullptr;
                break;
            case 1:
                newProcessor = std::make_unique<Effects::Equalizer::EqualizerProcessor>(apvts);
                break;
            case 2:
                newProcessor = std::make_unique<Effects::Filter::FilterProcessor>(apvts);
                break;
            case 3:
                newProcessor = std::make_unique<Effects::Compressor::CompressorProcessor>(apvts);
                break;
            case 4:
                newProcessor = std::make_unique<Effects::Delay::DelayProcessor>(apvts);
                break;
            case 5:
                newProcessor = std::make_unique<Effects::Reverb::ReverbProcessor>(apvts);
                break;
            case 6:
                newProcessor = std::make_unique<Effects::Chorus::ChorusProcessor>(apvts);
                break;
            case 7:
                newProcessor = std::make_unique<Effects::Phaser::PhaserProcessor>(apvts);
                break;
            case 8:
                newProcessor = std::make_unique<Effects::Tremolo::TremoloProcessor>(apvts);
                break;
            default:
                newProcessor = nullptr;
                break;
            }

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
    }
}