/*
==============================================================================

    PhaserUnit.cpp
    Created: 11 Apr 2023 9:39:34pm
    Author:  Habama10

==============================================================================
*/

#include "PhaserUnit.h"

#include "../../../../View/Effects/EffectEditors/Phaser/PhaserEditor.h"

namespace Effects::EffectProcessors::Phaser
{
    PhaserUnit::PhaserUnit(juce::AudioProcessorValueTreeState& apvts) : EffectProcessorUnit(apvts)
    {
        registerListener(this);
    }

    PhaserUnit::~PhaserUnit() 
    {
        removeListener(this);
    }

    void PhaserUnit::prepareToPlay(double sampleRate, int samplesPerBlock) 
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);

        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = samplesPerBlock;
        processSpec.numChannels = getTotalNumOutputChannels();
        processSpec.sampleRate = sampleRate;
        phaser.prepare(processSpec);
        updatePhaserParameters();
    }

    void PhaserUnit::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) 
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        
        juce::dsp::ProcessContextReplacing<float> context(audioBlock);

        phaser.process(context);
    }

    void PhaserUnit::releaseResources() 
    {
        phaser.reset();
    }

    void PhaserUnit::registerListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void PhaserUnit::removeListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void PhaserUnit::updatePhaserParameters()
    {
        phaser.setMix(apvts.getRawParameterValue("phaserMix")->load()/100);
        phaser.setRate(apvts.getRawParameterValue("phaserRate")->load());
        phaser.setDepth(apvts.getRawParameterValue("phaserDepth")->load()/100);
        phaser.setCentreFrequency(apvts.getRawParameterValue("phaserFrequency")->load());
        phaser.setFeedback(apvts.getRawParameterValue("phaserFeedback")->load()/100);
    }

    void PhaserUnit::parameterChanged(const juce::String &parameterID, float newValue) 
    {
        updatePhaserParameters();
    }
    
    std::unique_ptr<juce::AudioProcessorParameterGroup> PhaserUnit::createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> phaserGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>(
                "phaserGroup", 
                "Phaser", 
                "|"));

        auto mix = std::make_unique<juce::AudioParameterFloat>(
            "phaserMix", 
            "Wet%",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1),
            35.f);
        phaserGroup.get()->addChild(std::move(mix));

        auto rate = std::make_unique<juce::AudioParameterFloat>(
            "phaserRate", 
            "Rate",
            juce::NormalisableRange<float>(0.1, 20.f, 0.01, 0.35),
            2.f);
        phaserGroup.get()->addChild(std::move(rate));

        auto depth = std::make_unique<juce::AudioParameterFloat>(
            "phaserDepth", 
            "Depth",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            30.f);
        phaserGroup.get()->addChild(std::move(depth));

        auto frequency = std::make_unique<juce::AudioParameterFloat>(
            "phaserFrequency", 
            "Frequency",
            juce::NormalisableRange<float>(10.f, 22000.f, 0.1, 0.25),
            1000.f);
        phaserGroup.get()->addChild(std::move(frequency));

        auto feedback = std::make_unique<juce::AudioParameterFloat>(
            "phaserFeedback", 
            "Feedback",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1),
            40.f);
        phaserGroup.get()->addChild(std::move(feedback));

        return phaserGroup;
    }

    EffectEditorUnit* PhaserUnit::createEditorUnit()
    {
        return new PhaserEditor(apvts);
    }
}