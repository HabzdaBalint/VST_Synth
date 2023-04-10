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

namespace EffectsChain
{
    FXProcessorChain::FXProcessorChain(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
    {
        registerListeners();
        for (size_t i = 0; i < FX_MAX_SLOTS ; i++)
        {
            chain.add(nullptr);
            bypasses.push_back(false);
        }

        processors.add(nullptr);
        processors.add(std::make_unique<EffectProcessors::Equalizer::EqualizerUnit>(apvts));
        processors.add(std::make_unique<EffectProcessors::Filter::FilterUnit>(apvts));
        processors.add(std::make_unique<EffectProcessors::Compressor::CompressorUnit>(apvts));
        processors.add(std::make_unique<EffectProcessors::Delay::DelayUnit>(apvts));
        processors.add(std::make_unique<EffectProcessors::Reverb::ReverbUnit>(apvts));
        processors.add(std::make_unique<EffectProcessors::Chorus::ChorusUnit>(apvts));
        processors.add(std::make_unique<EffectProcessors::Phaser::PhaserUnit>(apvts));
        processors.add(std::make_unique<EffectProcessors::Tremolo::TremoloUnit>(apvts));
    }

    FXProcessorChain::~FXProcessorChain() {}

    void FXProcessorChain::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);
        for(auto processor : processors)
        {
            if( processor != nullptr )
                processor->prepareToPlay(sampleRate, samplesPerBlock);
        }
    }

    void FXProcessorChain::releaseResources()
    {
        for(auto processor : processors)
        {
            if( processor != nullptr )
                processor->releaseResources();
        }
    }

    void FXProcessorChain::processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &midiMessages)
    {
        for(size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            if ( !bypasses[i] && chain[i] != nullptr )
                chain[i]->processBlock(buffer, midiMessages);
        }
    }

    void FXProcessorChain::registerListeners()
    {
        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            apvts.addParameterListener(getFXBypassParameterName(i), this);
            apvts.addParameterListener(getFXChoiceParameterName(i), this);
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
        return "fxSlot" + juce::String(index);
    }

    void FXProcessorChain::parameterChanged(const juce::String &parameterID, float newValue)
    {
        if(parameterID.contains("fxBypass"))
        {
            int idx = std::stoi(parameterID.trimCharactersAtStart("fxBypass").toStdString());
            bypasses[idx] = (bool)newValue;
        }
        else
        {
            jassert(juce::isPositiveAndBelow(newValue, processors.size()));

            int idx = std::stoi(parameterID.trimCharactersAtStart("fxSlot").toStdString());
            
            chain.getReference(idx) = processors[(int)newValue];
        }
    }
}