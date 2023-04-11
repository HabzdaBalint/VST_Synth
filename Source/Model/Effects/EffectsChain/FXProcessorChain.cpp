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
            std::shared_ptr<Effects::EffectProcessors::EffectProcessorUnit> localProcessor = item->processor;
            if( localProcessor != nullptr )
                localProcessor->prepareToPlay(sampleRate, samplesPerBlock);
        }
    }

    void FXProcessorChain::releaseResources()
    {
        for(auto item : chain)
        {
            std::shared_ptr<Effects::EffectProcessors::EffectProcessorUnit> localProcessor = item->processor;
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
                std::shared_ptr<Effects::EffectProcessors::EffectProcessorUnit> localProcessor = item->processor;
                if( localProcessor != nullptr )
                    localProcessor->processBlock(buffer, midiMessages);
            }
        }
    }

    void FXProcessorChain::registerListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = FXProcessorChain::createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void FXProcessorChain::removeListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = FXProcessorChain::createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    std::unique_ptr<juce::AudioProcessorParameterGroup> FXProcessorChain::createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> fxChainGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("fxChainGroup", "Effect Chain", "|"));

        juce::AudioParameterChoiceAttributes attr;

        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            //Bypasses for each loaded Effect
            auto processorBypass = std::make_unique<juce::AudioParameterBool>(
                getFXBypassParameterName(i),
                "Bypass " + juce::String(i+1),
                false);
            fxChainGroup.get()->addChild(std::move(processorBypass));
            
            //Choices for each loaded Effect
            auto processorChoice = std::make_unique<juce::AudioParameterChoice>(
                getFXChoiceParameterName(i),
                "FX Slot " + juce::String(i+1),
                choices,
                0,
                attr.withAutomatable(false).withMeta(true));
            fxChainGroup.get()->addChild(std::move(processorChoice));
        }

        return fxChainGroup;
    }

    const juce::String FXProcessorChain::getFXBypassParameterName(size_t index)
    {
        return "fxBypass" + juce::String(index);
    }

    const juce::String FXProcessorChain::getFXChoiceParameterName(size_t index)
    {
        return "fxChoice" + juce::String(index);
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

            std::unique_ptr<Effects::EffectProcessors::EffectProcessorUnit> newProcessor;
            switch ((int)newValue)
            {
            case 0:
                newProcessor = nullptr;
                break;
            case 1:
                newProcessor = std::make_unique<Effects::EffectProcessors::Equalizer::EqualizerUnit>(apvts);
                break;
            case 2:
                newProcessor = std::make_unique<Effects::EffectProcessors::Filter::FilterUnit>(apvts);
                break;
            case 3:
                newProcessor = std::make_unique<Effects::EffectProcessors::Compressor::CompressorUnit>(apvts);
                break;
            case 4:
                newProcessor = std::make_unique<Effects::EffectProcessors::Delay::DelayUnit>(apvts);
                break;
            case 5:
                newProcessor = std::make_unique<Effects::EffectProcessors::Reverb::ReverbUnit>(apvts);
                break;
            case 6:
                newProcessor = std::make_unique<Effects::EffectProcessors::Chorus::ChorusUnit>(apvts);
                break;
            case 7:
                newProcessor = std::make_unique<Effects::EffectProcessors::Phaser::PhaserUnit>(apvts);
                break;
            case 8:
                newProcessor = std::make_unique<Effects::EffectProcessors::Tremolo::TremoloUnit>(apvts);
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