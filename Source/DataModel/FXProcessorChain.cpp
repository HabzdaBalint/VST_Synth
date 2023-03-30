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

FXProcessorChain::FXProcessorChain()
{
    for (size_t i = 0; i < FX_MAX_SLOTS; i++)
    {
        fxProcessorChain[i] = nullptr;
        bypassSlot[i] = false;
    }
}

FXProcessorChain::~FXProcessorChain()
{
    delete(equalizer);
    delete(filter);
    delete(compressor);
    delete(delay);
    delete(reverb);
    delete(chorus);
    delete(phaser);
    delete(tremolo);
}

juce::AudioProcessorEditor* FXProcessorChain::createEditor() 
{
    return nullptr;
}

double FXProcessorChain::getTailLengthSeconds() const
{
    return 0;
}  //todo

void FXProcessorChain::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    updateGraph();
}

void FXProcessorChain::releaseResources()
{
    for (size_t i = 0; i < FX_MAX_SLOTS; i++)
    {
        if( fxProcessorChain[i] != nullptr )
            fxProcessorChain[i]->releaseResources();
    }
}

void FXProcessorChain::processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &midiMessages)
{
    for (size_t i = 0; i < FX_MAX_SLOTS; i++)
    {
        if (!bypassSlot[i] && fxProcessorChain[i] != nullptr)
            fxProcessorChain[i]->processBlock(buffer, midiMessages);
    }
}

void FXProcessorChain::registerListeners()
{
    updateGraph();

    for (size_t i = 0; i < FX_MAX_SLOTS; i++)
    {
        apvts->addParameterListener(getFXBypassParameterName(i), this);
        apvts->addParameterListener(getFXChoiceParameterName(i), this);
    }
}

void FXProcessorChain::createParameters(std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> &layout)
{
    layout.push_back(createParameterLayout());

    layout.push_back(equalizer->createParameterLayout());
    layout.push_back(filter->createParameterLayout());
    layout.push_back(compressor->createParameterLayout());
    layout.push_back(delay->createParameterLayout());
    layout.push_back(reverb->createParameterLayout());
    layout.push_back(chorus->createParameterLayout());
    layout.push_back(phaser->createParameterLayout());
    layout.push_back(tremolo->createParameterLayout());
}

std::unique_ptr<juce::AudioProcessorParameterGroup> FXProcessorChain::createParameterLayout()
{
    std::unique_ptr<juce::AudioProcessorParameterGroup> fxGroup (
        std::make_unique<juce::AudioProcessorParameterGroup>("fxGroup", "Effects", "|"));

    for (size_t i = 0; i < FX_MAX_SLOTS; i++)
    {
        //Bypasses for each loaded Effect
        auto bypassProcessor = std::make_unique<juce::AudioParameterBool>(getFXBypassParameterName(i), "Bypass " + juce::String(i+1) , false);
        fxGroup.get()->addChild(std::move(bypassProcessor));
        
        //Choices for each loaded Effect
        auto choiceProcessor = std::make_unique<juce::AudioParameterChoice>(getFXChoiceParameterName(i), "FX Slot " + juce::String(i+1), choices, 0);
        fxGroup.get()->addChild(std::move(choiceProcessor));
    }

    return fxGroup;
}

void FXProcessorChain::connectApvts(juce::AudioProcessorValueTreeState& apvts)
{
    this->apvts = &apvts;
    registerListeners();

    equalizer->connectApvts(apvts);
    filter->connectApvts(apvts);
    compressor->connectApvts(apvts);
    delay->connectApvts(apvts);
    reverb->connectApvts(apvts);
    chorus->connectApvts(apvts);
    phaser->connectApvts(apvts);
    tremolo->connectApvts(apvts);
}

void FXProcessorChain::updateGraph()
{
    for (size_t i = 0; i < FX_MAX_SLOTS; i++)
    {
        juce::AudioParameterChoice* choice = dynamic_cast<juce::AudioParameterChoice*>(apvts->getParameter(getFXChoiceParameterName(i)));

        switch (choice->getIndex())
        {
        case 0:
            fxProcessorChain[i] = nullptr;
            break;
        case 1:
            fxProcessorChain[i] = equalizer;
            equalizer->setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), getSampleRate(), getBlockSize());
            equalizer->prepareToPlay(getSampleRate(), getBlockSize());
            break;
        case 2:
            fxProcessorChain[i] = filter;
            filter->setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), getSampleRate(), getBlockSize());
            filter->prepareToPlay(getSampleRate(), getBlockSize());
            break;
        case 3:
            fxProcessorChain[i] = compressor;
            compressor->setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), getSampleRate(), getBlockSize());
            compressor->prepareToPlay(getSampleRate(), getBlockSize());
            break;
        case 4:
            fxProcessorChain[i] = delay;
            delay->setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), getSampleRate(), getBlockSize());
            delay->prepareToPlay(getSampleRate(), getBlockSize());
            break;
        case 5:
            fxProcessorChain[i] = reverb;
            reverb->setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), getSampleRate(), getBlockSize());
            reverb->prepareToPlay(getSampleRate(), getBlockSize());
            break;
        case 6:
            fxProcessorChain[i] = chorus;
            chorus->setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), getSampleRate(), getBlockSize());
            chorus->prepareToPlay(getSampleRate(), getBlockSize());
            break;
        case 7:
            fxProcessorChain[i] = phaser;
            phaser->setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), getSampleRate(), getBlockSize());
            phaser->prepareToPlay(getSampleRate(), getBlockSize());
            break;
        case 8:
            fxProcessorChain[i] = tremolo;
            tremolo->setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), getSampleRate(), getBlockSize());
            tremolo->prepareToPlay(getSampleRate(), getBlockSize());
            break;
        default:
            break;
        }
    }

    for (size_t i = 0; i < FX_MAX_SLOTS; i++)
    {
        juce::AudioParameterBool* bypass = dynamic_cast<juce::AudioParameterBool*>(apvts->getParameter(getFXBypassParameterName(i)));
        bypassSlot[i] = bypass->get();
    }
}

void FXProcessorChain::parameterChanged(const juce::String &parameterID, float newValue)
{
    triggerAsyncUpdate();
}

void FXProcessorChain::handleAsyncUpdate()
{
    updateGraph();
}
