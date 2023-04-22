/*
==============================================================================

    CompressorUnit.cpp
    Created: 11 Apr 2023 9:38:29pm
    Author:  Habama10

==============================================================================
*/

#include "CompressorUnit.h"

#include "../../../View/Effects/EffectEditors/Compressor/CompressorEditor.h"

namespace Effects::Compressor
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

    EffectEditorUnit* CompressorUnit::createEditorUnit()
    {
        return new CompressorEditor(apvts);
    }
}
