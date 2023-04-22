/*
==============================================================================

    ReverbUnit.cpp
    Created: 11 Apr 2023 9:39:27pm
    Author:  Habama10

==============================================================================
*/

#include "ReverbUnit.h"

#include "../../../View/Effects/EffectEditors/Reverb/ReverbEditor.h"

namespace Effects::Reverb
{
    ReverbUnit::ReverbUnit(juce::AudioProcessorValueTreeState& apvts) : EffectProcessorUnit(apvts)
    {
        registerListener(this);
    }

    ReverbUnit::~ReverbUnit() 
    {
        removeListener(this);
    }

    void ReverbUnit::prepareToPlay(double sampleRate, int samplesPerBlock) 
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);

        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = samplesPerBlock;
        processSpec.numChannels = getTotalNumOutputChannels();
        processSpec.sampleRate = sampleRate;
        reverb.prepare(processSpec);
        updateReverbParameters();
    }

    void ReverbUnit::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) 
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        
        juce::dsp::ProcessContextReplacing<float> context(audioBlock);

        reverb.process(context);
    }

    void ReverbUnit::releaseResources() 
    {
        reverb.reset();
    }

    void ReverbUnit::registerListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void ReverbUnit::removeListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void ReverbUnit::updateReverbParameters()
    {
        Reverb::Parameters newParams;
        newParams.wetLevel = apvts.getRawParameterValue("reverbWet")->load()/100;
        newParams.dryLevel = apvts.getRawParameterValue("reverbDry")->load()/100;
        newParams.roomSize = apvts.getRawParameterValue("reverbRoom")->load()/100;
        newParams.damping = apvts.getRawParameterValue("reverbDamping")->load()/100;
        newParams.width = apvts.getRawParameterValue("reverbWidth")->load()/100;
        reverb.setParameters(newParams);
    }        
    
    void ReverbUnit::parameterChanged(const juce::String &parameterID, float newValue) 
    {
        updateReverbParameters();
    }
    
    EffectEditorUnit* ReverbUnit::createEditorUnit()
    {
        return new ReverbEditor(apvts);
    }
}