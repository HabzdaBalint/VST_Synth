/*
==============================================================================

    ChorusUnit.cpp
    Created: 11 Apr 2023 9:34:36pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "ChorusUnit.h"

#include "../../../../View/Effects/EffectEditors/Chorus/ChorusEditor.h"

namespace Effects::EffectProcessors::Chorus
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
    
    std::unique_ptr<juce::AudioProcessorParameterGroup> ChorusUnit::createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> chorusGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>(
                "chorusGroup", 
                "Chorus", 
                "|"));

        auto mix = std::make_unique<juce::AudioParameterFloat>(
            "chorusMix", 
            "Wet%",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            35.f);
        chorusGroup.get()->addChild(std::move(mix));

        auto rate = std::make_unique<juce::AudioParameterFloat>(
            "chorusRate", 
            "Rate",
            juce::NormalisableRange<float>(0.1, 20.f, 0.01, 0.35), 
            3.f);
        chorusGroup.get()->addChild(std::move(rate));

        auto delay = std::make_unique<juce::AudioParameterFloat>(
            "chorusDelay", 
            "Delay",
            juce::NormalisableRange<float>(1.f, 50.f, 0.1), 
            20.f);
        chorusGroup.get()->addChild(std::move(delay));

        auto depth = std::make_unique<juce::AudioParameterFloat>(
            "chorusDepth", 
            "Depth",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            20.f);
        chorusGroup.get()->addChild(std::move(depth));

        auto feedback = std::make_unique<juce::AudioParameterFloat>(
            "chorusFeedback", 
            "Feedback",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            40.f);
        chorusGroup.get()->addChild(std::move(feedback));

        return chorusGroup;
    }

    EffectEditorUnit* ChorusUnit::createEditorUnit()
    {
        return new ChorusEditor(apvts);
    }
}