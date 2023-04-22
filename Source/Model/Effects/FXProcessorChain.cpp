/*
==============================================================================

FXProcessorChain.cpp
Created: 26 Mar 2023 8:56:05pm
Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FXProcessorChain.h"

namespace Effects::EffectsChain
{
    FXProcessorChain::FXProcessorChain(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
    {
        for (size_t i = 0; i < FX_MAX_SLOTS ; i++)
        {
            chain.add(std::make_unique<FXProcessor>());
        }

        registerListener(this);
    }

    FXProcessorChain::~FXProcessorChain()
    {
        removeListener(this);
    }

    void FXProcessorChain::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);
        for(auto item : chain)
        {
            std::shared_ptr<Effects::EffectProcessorUnit> localProcessor = item->processor;
            if( localProcessor != nullptr )
                localProcessor->prepareToPlay(sampleRate, samplesPerBlock);
        }
    }

    void FXProcessorChain::releaseResources()
    {
        for(auto item : chain)
        {
            std::shared_ptr<Effects::EffectProcessorUnit> localProcessor = item->processor;
            if( localProcessor != nullptr )
                localProcessor->releaseResources();
        }
    }

    void FXProcessorChain::processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &midiMessages)
    {
        for(auto item : chain)
        {
            if( !item->bypass )
            {
                std::shared_ptr<Effects::EffectProcessorUnit> localProcessor = item->processor;
                if( localProcessor != nullptr )
                    localProcessor->processBlock(buffer, midiMessages);
            }
        }
    }

    void FXProcessorChain::registerListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void FXProcessorChain::removeListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    juce::Array<EffectEditorUnit*> FXProcessorChain::getLoadedEffectEditors()
    {
        juce::Array<EffectEditorUnit*> editorComponents;

        for(auto item : chain)
        {
            if( item->processor.load() )
                editorComponents.add( item->processor.load()->createEditorUnit() );
            else
                editorComponents.add( nullptr );
        }

        return editorComponents;
    }

    void FXProcessorChain::parameterChanged(const juce::String &parameterID, float newValue)
    {
        if(parameterID.contains("fxBypass"))
        {
            int idx = std::stoi(parameterID.trimCharactersAtStart("fxBypass").toStdString());
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

            int idx = std::stoi(parameterID.trimCharactersAtStart("fxChoice").toStdString());

            std::unique_ptr<Effects::EffectProcessorUnit> newProcessor;
            switch ((int)newValue)
            {
            case 0:
                newProcessor = nullptr;
                break;
            case 1:
                newProcessor = std::make_unique<Effects::Equalizer::EqualizerUnit>(apvts);
                break;
            case 2:
                newProcessor = std::make_unique<Effects::Filter::FilterUnit>(apvts);
                break;
            case 3:
                newProcessor = std::make_unique<Effects::Compressor::CompressorUnit>(apvts);
                break;
            case 4:
                newProcessor = std::make_unique<Effects::Delay::DelayUnit>(apvts);
                break;
            case 5:
                newProcessor = std::make_unique<Effects::Reverb::ReverbUnit>(apvts);
                break;
            case 6:
                newProcessor = std::make_unique<Effects::Chorus::ChorusUnit>(apvts);
                break;
            case 7:
                newProcessor = std::make_unique<Effects::Phaser::PhaserUnit>(apvts);
                break;
            case 8:
                newProcessor = std::make_unique<Effects::Tremolo::TremoloUnit>(apvts);
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