/*
==============================================================================

    ReverbUnit.cpp
    Created: 11 Apr 2023 9:39:27pm
    Author:  Habama10

==============================================================================
*/

#include "ReverbUnit.h"

#include "../../../../View/Effects/EffectEditors/Reverb/ReverbEditor.h"

namespace Effects::EffectProcessors::Reverb
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

    std::unique_ptr<juce::AudioProcessorParameterGroup> ReverbUnit::createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> reverbGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>(
                "reverbGroup", 
                "Reverb", 
                "|"));

        auto wetLevel = std::make_unique<juce::AudioParameterFloat>(
            "reverbWet", 
            "Wet%",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            20.f);
        reverbGroup.get()->addChild(std::move(wetLevel));

        auto dryLevel = std::make_unique<juce::AudioParameterFloat>(
            "reverbDry", 
            "Dry%",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            80.f);
        reverbGroup.get()->addChild(std::move(dryLevel));

        auto roomSize = std::make_unique<juce::AudioParameterFloat>(
            "reverbRoom", 
            "Room Size",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            50.f);
        reverbGroup.get()->addChild(std::move(roomSize));

        auto damping = std::make_unique<juce::AudioParameterFloat>(
            "reverbDamping", 
            "Damping",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            50.f);
        reverbGroup.get()->addChild(std::move(damping));

        auto width = std::make_unique<juce::AudioParameterFloat>(
            "reverbWidth", 
            "Width",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            50.f);
        reverbGroup.get()->addChild(std::move(width));

        return reverbGroup;
    }
    
    EffectEditorUnit* ReverbUnit::createEditorUnit()
    {
        return new ReverbEditor(apvts);
    }
}