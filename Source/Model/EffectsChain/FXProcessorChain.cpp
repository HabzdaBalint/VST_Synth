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
        processors.add(std::make_unique<EffectProcessors::EqualizerUnit>(apvts));
        processors.add(std::make_unique<EffectProcessors::FilterUnit>(apvts));
        processors.add(std::make_unique<EffectProcessors::CompressorUnit>(apvts));
        processors.add(std::make_unique<EffectProcessors::DelayUnit>(apvts));
        processors.add(std::make_unique<EffectProcessors::ReverbUnit>(apvts));
        processors.add(std::make_unique<EffectProcessors::ChorusUnit>(apvts));
        processors.add(std::make_unique<EffectProcessors::PhaserUnit>(apvts));
        processors.add(std::make_unique<EffectProcessors::TremoloUnit>(apvts));
    }

    FXProcessorChain::~FXProcessorChain() {}

    double FXProcessorChain::getTailLengthSeconds() const
    {
        double tailLength = 0.f;
        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            if( chain[i] != nullptr )
                tailLength += chain[i]->getTailLengthSeconds();
        }
        return tailLength;
    }

    void FXProcessorChain::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);
        for (size_t i = 0; i < processors.size(); i++)
        {
            if( processors[i] != nullptr )
                processors[i]->prepareToPlay(sampleRate, samplesPerBlock);
        }
    }

    void FXProcessorChain::releaseResources()
    {
        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            if( processors[i] != nullptr )
                processors[i]->releaseResources();
        }
    }

    void FXProcessorChain::processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &midiMessages)
    {
        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            if (!bypasses[i] && chain[i] != nullptr)
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
            auto bypassProcessor = std::make_unique<juce::AudioParameterBool>(
                getFXBypassParameterName(i),
                "Bypass " + juce::String(i+1),
                false);
            fxChainGroup.get()->addChild(std::move(bypassProcessor));
            
            //Choices for each loaded Effect
            auto choiceProcessor = std::make_unique<juce::AudioParameterChoice>(
                getFXChoiceParameterName(i),
                "FX Slot " + juce::String(i+1),
                choices,
                0,
                attr.withAutomatable(false).withMeta(true));
            fxChainGroup.get()->addChild(std::move(choiceProcessor));
        }

        return fxChainGroup;
    }

    void FXProcessorChain::parameterChanged(const juce::String &parameterID, float newValue)
    {
        if(parameterID.contains("bypass"))
        {
            int idx = std::stoi(parameterID.trimCharactersAtStart("bypass").toStdString());
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