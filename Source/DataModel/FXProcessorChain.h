#pragma once

#include <JuceHeader.h>
#include "FXProcessorBase.h"

class FXProcessorChain : FXProcessorBase
{

public:
    FXProcessorChain()
    {
        initialize();
    }

    ~FXProcessorChain() {}

    const juce::String getName() const override { return "Effect Chain"; }

    juce::AudioProcessorEditor* createEditor() override { return nullptr; } //todo return the fx chain's editor object

    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        fxProcessorChain.setPlayConfigDetails (getMainBusNumInputChannels(),
                                                getMainBusNumOutputChannels(),
                                                sampleRate, samplesPerBlock);

        fxProcessorChain.prepareToPlay (sampleRate, samplesPerBlock);

        initialize();
    }

    void releaseResources() override
    {
        fxProcessorChain.releaseResources();
    }

    void processBlock (juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiMessages) override
    {
        for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
            buffer.clear (i, 0, buffer.getNumSamples());

        updateGraph();

        fxProcessorChain.processBlock (buffer, midiMessages);
    }

private:
    juce::AudioProcessorGraph fxProcessorChain;

    juce::AudioProcessorGraph::Node::Ptr audioInputNode;
    juce::AudioProcessorGraph::Node::Ptr audioOutputNode;
    juce::AudioProcessorGraph::Node::Ptr midiInputNode;
    juce::AudioProcessorGraph::Node::Ptr midiOutputNode;

    void initialize()
    {
        fxProcessorChain.clear();
        audioInputNode = fxProcessorChain.addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode));
        audioOutputNode = fxProcessorChain.addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode));
        midiInputNode = fxProcessorChain.addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode));
        midiOutputNode  = fxProcessorChain.addNode (std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(juce::AudioProcessorGraph::AudioGraphIOProcessor::midiOutputNode));

        connectAudioNodes();
        connectMidiNodes();
    }

    void connectAudioNodes()
    {
        for (int channel = 0; channel < 2; channel++)
        {
            fxProcessorChain.addConnection({ { audioInputNode->nodeID,  channel }, 
                                             { audioOutputNode->nodeID, channel } });
        }
    }

    void connectMidiNodes()
    {
        fxProcessorChain.addConnection({ { midiInputNode->nodeID,  juce::AudioProcessorGraph::midiChannelIndex },
                                         { midiOutputNode->nodeID, juce::AudioProcessorGraph::midiChannelIndex } });
    }

    void updateGraph()
    {
        
    }
};