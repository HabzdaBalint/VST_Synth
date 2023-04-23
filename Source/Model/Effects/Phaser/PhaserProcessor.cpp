/*
==============================================================================

    PhaserProcessor.cpp
    Created: 11 Apr 2023 9:39:34pm
    Author:  Habama10

==============================================================================
*/

#include "PhaserProcessor.h"

#include "../../../View/Effects/Phaser/PhaserEditor.h"

namespace Effects::Phaser
{
    PhaserProcessor::PhaserProcessor(juce::AudioProcessorValueTreeState& apvts) : EffectProcessor(apvts)
    {
        registerListener(this);
    }

    PhaserProcessor::~PhaserProcessor() 
    {
        removeListener(this);
    }

    void PhaserProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) 
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);

        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = samplesPerBlock;
        processSpec.numChannels = getTotalNumOutputChannels();
        processSpec.sampleRate = sampleRate;
        phaser.prepare(processSpec);
        updatePhaserParameters();
    }

    void PhaserProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) 
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        
        juce::dsp::ProcessContextReplacing<float> context(audioBlock);

        phaser.process(context);
    }

    void PhaserProcessor::releaseResources() 
    {
        phaser.reset();
    }

    void PhaserProcessor::registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void PhaserProcessor::removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void PhaserProcessor::updatePhaserParameters()
    {
        phaser.setMix(apvts.getRawParameterValue("phaserMix")->load()/100);
        phaser.setRate(apvts.getRawParameterValue("phaserRate")->load());
        phaser.setDepth(apvts.getRawParameterValue("phaserDepth")->load()/100);
        phaser.setCentreFrequency(apvts.getRawParameterValue("phaserFrequency")->load());
        phaser.setFeedback(apvts.getRawParameterValue("phaserFeedback")->load()/100);
    }

    void PhaserProcessor::parameterChanged(const juce::String &parameterID, float newValue) 
    {
        updatePhaserParameters();
    }
    
    EffectEditor* PhaserProcessor::createEditorUnit()
    {
        return new PhaserEditor(apvts);
    }
}