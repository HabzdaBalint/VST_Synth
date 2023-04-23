/*
==============================================================================

    ChorusProcessor.cpp
    Created: 11 Apr 2023 9:34:36pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "ChorusProcessor.h"

#include "../../../View/Effects/Chorus/ChorusEditor.h"

namespace Effects::Chorus
{
    ChorusProcessor::ChorusProcessor(juce::AudioProcessorValueTreeState& apvts) : EffectProcessor(apvts)
    {
        registerListener(this);
    }

    ChorusProcessor::~ChorusProcessor()
    {
        removeListener(this);
    }

    void ChorusProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);

        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = samplesPerBlock;
        processSpec.numChannels = getTotalNumOutputChannels();
        processSpec.sampleRate = sampleRate;
        chorus.prepare(processSpec);
        updateChorusParameters();
    }

    void ChorusProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        
        juce::dsp::ProcessContextReplacing<float> context(audioBlock);

        chorus.process(context);
    }

    void ChorusProcessor::releaseResources()
    {
        chorus.reset();
    }

    void ChorusProcessor::registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void ChorusProcessor::removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void ChorusProcessor::updateChorusParameters()
    {
        chorus.setMix(apvts.getRawParameterValue("chorusMix")->load()/100);
        chorus.setRate(apvts.getRawParameterValue("chorusRate")->load());
        chorus.setCentreDelay(apvts.getRawParameterValue("chorusDelay")->load());
        chorus.setDepth(apvts.getRawParameterValue("chorusDepth")->load()/100);
        chorus.setFeedback(apvts.getRawParameterValue("chorusFeedback")->load()/100);
    }

    void ChorusProcessor::parameterChanged(const juce::String &parameterID, float newValue)
    {
        updateChorusParameters();
    }

    EffectEditor* ChorusProcessor::createEditorUnit()
    {
        return new ChorusEditor(apvts);
    }
}