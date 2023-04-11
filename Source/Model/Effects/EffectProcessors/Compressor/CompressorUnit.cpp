/*
==============================================================================

    CompressorUnit.cpp
    Created: 11 Apr 2023 9:38:29pm
    Author:  Habama10

==============================================================================
*/

#include "CompressorUnit.h"

#include "../../../../View/Effects/EffectEditors/Compressor/CompressorEditor.h"

namespace Effects::EffectProcessors::Compressor
{
    CompressorUnit::CompressorUnit(juce::AudioProcessorValueTreeState& apvts) : EffectProcessorUnit(apvts)
    {
        dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
        registerListener(this);
    }

    CompressorUnit::~CompressorUnit()  
    {
        removeListener(this);
    }

    void CompressorUnit::prepareToPlay(double sampleRate, int samplesPerBlock) 
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);

        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = samplesPerBlock;
        processSpec.numChannels = getTotalNumOutputChannels();
        processSpec.sampleRate = sampleRate;
        dryWetMixer.prepare(processSpec);
        compressor.prepare(processSpec);
        updateCompressorParameters();
    }

    void CompressorUnit::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) 
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> context(audioBlock);

        dryWetMixer.pushDrySamples(audioBlock);
        compressor.process(context);
        dryWetMixer.mixWetSamples(audioBlock);
    }

    void CompressorUnit::releaseResources() 
    {
        dryWetMixer.reset();
        compressor.reset();
    }

    void CompressorUnit::registerListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void CompressorUnit::removeListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void CompressorUnit::updateCompressorParameters()
    {
        dryWetMixer.setWetMixProportion(apvts.getRawParameterValue("compressorMix")->load()/100);
        compressor.setThreshold(apvts.getRawParameterValue("compressorThreshold")->load());
        compressor.setRatio(apvts.getRawParameterValue("compressorRatio")->load());
        compressor.setAttack(apvts.getRawParameterValue("compressorAttack")->load());
        compressor.setRelease(apvts.getRawParameterValue("compressorRelease")->load());
    }

    void CompressorUnit::parameterChanged(const juce::String &parameterID, float newValue) 
    {
        updateCompressorParameters();
    }
    
    std::unique_ptr<juce::AudioProcessorParameterGroup> CompressorUnit::createParameterLayout()
    {
        std::unique_ptr<juce::AudioProcessorParameterGroup> compressorGroup (
            std::make_unique<juce::AudioProcessorParameterGroup>(
                "compressorGroup", 
                "Compressor", 
                "|"));

        auto dryWetMix = std::make_unique<juce::AudioParameterFloat>(
            "compressorMix",
            "Wet%",
            juce::NormalisableRange<float>(0.f, 100.f, 0.1), 
            100.f);
        compressorGroup.get()->addChild(std::move(dryWetMix));

        auto threshold = std::make_unique<juce::AudioParameterFloat>(
            "compressorThreshold",
            "Threshold",
            juce::NormalisableRange<float>(-60.f, 0.f, 0.01), 
            -18.f);
        compressorGroup.get()->addChild(std::move(threshold));

        juce::AudioParameterFloatAttributes attr;
        auto ratio = std::make_unique<juce::AudioParameterFloat>(
            "compressorRatio",
            "Ratio",
            juce::NormalisableRange<float>(1.f, 100.f, 0.01, 0.3), 
            4.f,
            attr.withStringFromValueFunction([](float value, int maximumStringLength)
                {
                    juce::String string = juce::String(value) + ":1";
                    return string;
                }));
        compressorGroup.get()->addChild(std::move(ratio));

        auto attack = std::make_unique<juce::AudioParameterFloat>(
            "compressorAttack",
            "Attack",
            juce::NormalisableRange<float>(0.01, 50.f, 0.01, 0.5), 
            0.5);
        compressorGroup.get()->addChild(std::move(attack));

        auto release = std::make_unique<juce::AudioParameterFloat>(
            "compressorRelease",
            "Release",
            juce::NormalisableRange<float>(1.f, 1000.f, 0.1, 0.5), 
            100.f);
        compressorGroup.get()->addChild(std::move(release));

        return compressorGroup;
    }

    EffectEditorUnit* CompressorUnit::createEditorUnit()
    {
        return new CompressorEditor(apvts);
    }
}
