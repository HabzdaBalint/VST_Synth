/*
==============================================================================

    ReverbProcessor.cpp
    Created: 11 Apr 2023 9:39:27pm
    Author:  Habama10

==============================================================================
*/

#include "ReverbProcessor.h"

#include "../../../View/Effects/Reverb/ReverbEditor.h"

namespace Processor::Effects::Reverb
{
    ReverbProcessor::ReverbProcessor(juce::AudioProcessorValueTreeState& apvts) : EffectProcessor(apvts)
    {
        registerListener(this);
    }

    ReverbProcessor::~ReverbProcessor() 
    {
        removeListener(this);
    }

    void ReverbProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) 
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);

        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = samplesPerBlock;
        processSpec.numChannels = getTotalNumOutputChannels();
        processSpec.sampleRate = sampleRate;
        reverb.prepare(processSpec);
        updateReverbParameters();
    }

    void ReverbProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) 
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        
        juce::dsp::ProcessContextReplacing<float> context(audioBlock);

        reverb.process(context);
    }

    void ReverbProcessor::releaseResources() 
    {
        reverb.reset();
    }

    void ReverbProcessor::registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void ReverbProcessor::removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void ReverbProcessor::updateReverbParameters()
    {
        Reverb::Parameters newParams;
        newParams.wetLevel = apvts.getRawParameterValue("reverbWet")->load()/100;
        newParams.dryLevel = apvts.getRawParameterValue("reverbDry")->load()/100;
        newParams.roomSize = apvts.getRawParameterValue("reverbRoom")->load()/100;
        newParams.damping = apvts.getRawParameterValue("reverbDamping")->load()/100;
        newParams.width = apvts.getRawParameterValue("reverbWidth")->load()/100;
        reverb.setParameters(newParams);
    }        
    
    void ReverbProcessor::parameterChanged(const juce::String &parameterID, float newValue) 
    {
        updateReverbParameters();
    }
    
    Editor::Effects::EffectEditor* ReverbProcessor::createEditorUnit()
    {
        return new Editor::Effects::ReverbEditor(apvts);
    }
}