/*
==============================================================================

    ChorusUnit.cpp
    Created: 11 Apr 2023 9:34:36pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "ChorusUnit.h"

#include "../../../View/Effects/EffectEditors/Chorus/ChorusEditor.h"

namespace Effects::Chorus
{
    ChorusUnit::ChorusUnit(juce::AudioProcessorValueTreeState& apvts) : EffectProcessorUnit(apvts)
    {
        registerListener(this);
    }

    ChorusUnit::~ChorusUnit()
    {
        removeListener(this);
    }

    void ChorusUnit::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);

        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = samplesPerBlock;
        processSpec.numChannels = getTotalNumOutputChannels();
        processSpec.sampleRate = sampleRate;
        chorus.prepare(processSpec);
        updateChorusParameters();
    }

    void ChorusUnit::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        
        juce::dsp::ProcessContextReplacing<float> context(audioBlock);

        chorus.process(context);
    }

    void ChorusUnit::releaseResources()
    {
        chorus.reset();
    }

    void ChorusUnit::registerListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void ChorusUnit::removeListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void ChorusUnit::updateChorusParameters()
    {
        chorus.setMix(apvts.getRawParameterValue("chorusMix")->load()/100);
        chorus.setRate(apvts.getRawParameterValue("chorusRate")->load());
        chorus.setCentreDelay(apvts.getRawParameterValue("chorusDelay")->load());
        chorus.setDepth(apvts.getRawParameterValue("chorusDepth")->load()/100);
        chorus.setFeedback(apvts.getRawParameterValue("chorusFeedback")->load()/100);
    }

    void ChorusUnit::parameterChanged(const juce::String &parameterID, float newValue)
    {
        updateChorusParameters();
    }

    EffectEditorUnit* ChorusUnit::createEditorUnit()
    {
        return new ChorusEditor(apvts);
    }
}