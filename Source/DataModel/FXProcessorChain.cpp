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

namespace FXChain
{
    FXProcessorChain::FXProcessorChain(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts)
    {
        registerListeners();
        for (size_t i = 0; i < FX_MAX_SLOTS ; i++)
        {
            processors.push_back(nullptr);
            bypasses.push_back(false);
        }
    }

    FXProcessorChain::~FXProcessorChain() {}

    double FXProcessorChain::getTailLengthSeconds() const
    {
        return 0;
    }  //todo

    void FXProcessorChain::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
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
            if (!bypasses[i] && processors[i] != nullptr)
                processors[i]->processBlock(buffer, midiMessages);
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
        std::unique_ptr<juce::AudioProcessorParameterGroup> fxGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>("fxGroup", "Effects", "|"));

        juce::AudioParameterChoiceAttributes attr;

        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            //Bypasses for each loaded Effect
            auto bypassProcessor = std::make_unique<juce::AudioParameterBool>(
                getFXBypassParameterName(i),
                "Bypass " + juce::String(i+1),
                false);
            fxGroup.get()->addChild(std::move(bypassProcessor));
            
            //Choices for each loaded Effect
            auto choiceProcessor = std::make_unique<juce::AudioParameterChoice>(
                getFXChoiceParameterName(i),
                "FX Slot " + juce::String(i+1),
                choices,
                0,
                attr.withAutomatable(false).withMeta(true));
            fxGroup.get()->addChild(std::move(choiceProcessor));
        }

        return fxGroup;
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
            int idx = std::stoi(parameterID.trimCharactersAtStart("fxSlot").toStdString());

            switch((int)newValue)
            {
            case 0:
                processors[idx] = nullptr;
                break;
            case 1:
                processors[idx] = std::make_unique<FXEqualizer>(apvts);
                break;
            case 2:
                processors[idx] = std::make_unique<FXFilter>(apvts);
                break;
            case 3:
                processors[idx] = std::make_unique<FXCompressor>(apvts);
                break;
            case 4:
                processors[idx] = std::make_unique<FXDelay>(apvts);
                break;
            case 5:
                processors[idx] = std::make_unique<FXReverb>(apvts);
                break;
            case 6:
                processors[idx] = std::make_unique<FXChorus>(apvts);
                break;
            case 7:
                processors[idx] = std::make_unique<FXPhaser>(apvts);
                break;
            case 8:
                processors[idx] = std::make_unique<FXTremolo>(apvts);
                break;
            default:
                break;
            }
            if(processors[idx] != nullptr)
            {
                processors[idx]->setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), getSampleRate(), getBlockSize());
                processors[idx]->prepareToPlay(getSampleRate(), getBlockSize());
            }
        }
    }
} // namespace FXChain
