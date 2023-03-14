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

class FXProcessorChain : FXProcessorBase
{
public:
    FXProcessorChain()
    {
        initialize();
    }

    ~FXProcessorChain() {}

    const juce::String getName() const override { return "Effect Chain"; }

    juce::AudioProcessorEditor *createEditor() override { return nullptr; } // todo return the fx chain's editor object

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        fxProcessorChain->setPlayConfigDetails(getMainBusNumInputChannels(),
                                              getMainBusNumOutputChannels(),
                                              sampleRate, samplesPerBlock);

        fxProcessorChain->prepareToPlay(sampleRate, samplesPerBlock);

        initialize();
    }

    void releaseResources() override
    {
        fxProcessorChain->releaseResources();
    }

    void processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &midiMessages) override
    {
        //todo potentially move to a listener instead of the audio callback
        updateGraph();

        fxProcessorChain->processBlock(buffer, midiMessages);
    }

    
    FXProcessorChainParameters chainParameters{[this]()
                                               { updateGraph(); }};

    //todo create instances of the various effects

private:
    std::unique_ptr<juce::AudioProcessorGraph> fxProcessorChain;

    juce::AudioProcessorGraph::Node::Ptr audioInputNode = nullptr;
    juce::AudioProcessorGraph::Node::Ptr audioOutputNode = nullptr;
    juce::AudioProcessorGraph::Node::Ptr midiInputNode = nullptr;
    juce::AudioProcessorGraph::Node::Ptr midiOutputNode = nullptr;

    juce::AudioProcessorGraph::Node::Ptr slotNode[FX_MAX_SLOTS] = {};

    void initialize()
    {
        fxProcessorChain->clear();
        audioInputNode = fxProcessorChain->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode));
        audioOutputNode = fxProcessorChain->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode));
        midiInputNode = fxProcessorChain->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode));
        midiOutputNode = fxProcessorChain->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::midiOutputNode));

        connectAudioNodes();
        connectMidiNodes();

        updateGraph();
    }

    void connectAudioNodes()
    {
        for (int channel = 0; channel < 2; channel++)
        {
            fxProcessorChain->addConnection({{audioInputNode->nodeID, channel},
                                            {audioOutputNode->nodeID, channel}});
        }
    }

    void connectMidiNodes()
    {
        fxProcessorChain->addConnection({{midiInputNode->nodeID, juce::AudioProcessorGraph::midiChannelIndex},
                                        {midiOutputNode->nodeID, juce::AudioProcessorGraph::midiChannelIndex}});
    }

    //todo
    void updateGraph()
    {
        bool graphChanged = false;

        for (size_t i = 0; i < FX_MAX_SLOTS; i++)
        {
            switch (chainParameters.choiceProcessor[i]->getIndex())
            {
            case 0: //Empty
                if (slotNode[i] != nullptr)
                {
                    graphChanged = true;
                    fxProcessorChain->removeNode(slotNode[i].get());
                }
                break;
            case 1:
                if (slotNode[i]->getProcessor()->getName() != /*equalizer.getName()*/ "Equalizer")
                {
                    graphChanged = true;
                    fxProcessorChain->addNode(std::make_unique<FXEqualizer>());
                    /* code */
                }
                
                break;
            
            default:
                break;
            }   
        }
        
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXProcessorChain)
};