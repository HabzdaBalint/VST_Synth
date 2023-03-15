/*
==============================================================================

    FXProcessorChainParameters.h
    Created: 12 Mar 2023 07:32:57pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FXProcessorBase.h"
#include "FXProcessorChainParameters.h"
#include "FXEqualizer.h"
#include "FXFilter.h"
#include "FXCompressor.h"
#include "FXDelay.h"
#include "FXReverb.h"
#include "FXChorus.h"
#include "FXPhaser.h"

class FXProcessorChain : public juce::AudioProcessor
{
public:
    FXProcessorChain()
    {
        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            fxProcessorChain[i] = nullptr;
            bypassSlot[i] = false;
        }
    }

    ~FXProcessorChain() {}

    const juce::String getName() const override { return "Effect Chain"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }

    juce::AudioProcessorEditor* createEditor() override { return nullptr; } // todo return the fx chain's editor object
    bool hasEditor() const override { return true; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    double getTailLengthSeconds() const override { return 0; }  //todo

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            if (fxProcessorChain[i] != nullptr)
            {
                fxProcessorChain[i]->setPlayConfigDetails(getMainBusNumInputChannels(),
                    getMainBusNumOutputChannels(),
                    sampleRate, samplesPerBlock);

                fxProcessorChain[i]->prepareToPlay(sampleRate, samplesPerBlock);
            }
        }

        updateGraph();
    }

    void releaseResources() override
    {
        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            if( fxProcessorChain[i] != nullptr )
                fxProcessorChain[i]->releaseResources();
        }
    }

    void processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &midiMessages) override
    {
        //todo potentially move to a listener instead of the audio callback
        updateGraph();

        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            if( !bypassSlot[i] && fxProcessorChain[i] != nullptr )
                fxProcessorChain[i]->processBlock(buffer, midiMessages);
        }
    }

    void registerListeners(juce::AudioProcessorValueTreeState &apvts)
    {
        chainParameters.update();

        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            apvts.addParameterListener(chainParameters.getFXBypassParameterName(i), &chainParameters);
            apvts.addParameterListener(chainParameters.getFXChoiceParameterName(i), &chainParameters);
        }
    }

    FXProcessorChainParameters chainParameters{[this]()
                                               { updateGraph(); }};

    FXEqualizer* equalizer = nullptr;
    FXFilter* filter = nullptr;
    FXCompressor* compressor = nullptr;
    FXDelay* delay = nullptr;
    FXReverb* reverb = nullptr;
    FXChorus* chorus = nullptr;
    FXPhaser* phaser = nullptr;
private:
    juce::AudioProcessor* fxProcessorChain[FX_MAX_SLOTS] = {};
    bool bypassSlot[FX_MAX_SLOTS] = {};

    juce::AudioProcessorGraph::Node::Ptr slotNode[FX_MAX_SLOTS] = {};

    void updateGraph()
    {
        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            switch (chainParameters.choiceProcessor[i]->getIndex())
            {
            case 0:
                fxProcessorChain[i] = nullptr;
                break;
            case 1:
                fxProcessorChain[i] = equalizer;
                break;
            case 2:
                fxProcessorChain[i] = filter;
                break;
            case 3:
                fxProcessorChain[i] = compressor;
                break;
            case 4:
                fxProcessorChain[i] = delay;
                break;
            case 5:
                fxProcessorChain[i] = reverb;
                break;
            case 6:
                fxProcessorChain[i] = chorus;
                break;
            case 7:
                fxProcessorChain[i] = phaser;
                break;
            default:
                break;
            }
        }

        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            bypassSlot[i] = chainParameters.bypassProcessor[i]->get();
        }
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXProcessorChain)
};