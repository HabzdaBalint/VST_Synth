/*
==============================================================================

    CompressorProcessor.cpp
    Created: 11 Apr 2023 9:38:29pm
    Author:  Habama10

==============================================================================
*/

#include "CompressorProcessor.h"

#include "../../../View/Effects/Compressor/CompressorEditor.h"

namespace Effects::Compressor
{
    CompressorProcessor::CompressorProcessor(juce::AudioProcessorValueTreeState& apvts) : EffectProcessor(apvts)
    {
        dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
        registerListener(this);
    }

    CompressorProcessor::~CompressorProcessor()  
    {
        removeListener(this);
    }

    void CompressorProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) 
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

    void CompressorProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) 
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> context(audioBlock);

        dryWetMixer.pushDrySamples(audioBlock);
        compressor.process(context);
        dryWetMixer.mixWetSamples(audioBlock);
    }

    void CompressorProcessor::releaseResources() 
    {
        dryWetMixer.reset();
        compressor.reset();
    }

    void CompressorProcessor::registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void CompressorProcessor::removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void CompressorProcessor::updateCompressorParameters()
    {
        dryWetMixer.setWetMixProportion(apvts.getRawParameterValue("compressorMix")->load()/100);
        compressor.setThreshold(apvts.getRawParameterValue("compressorThreshold")->load());
        compressor.setRatio(apvts.getRawParameterValue("compressorRatio")->load());
        compressor.setAttack(apvts.getRawParameterValue("compressorAttack")->load());
        compressor.setRelease(apvts.getRawParameterValue("compressorRelease")->load());
    }

    void CompressorProcessor::parameterChanged(const juce::String &parameterID, float newValue) 
    {
        updateCompressorParameters();
    }

    EffectEditor* CompressorProcessor::createEditorUnit()
    {
        return new CompressorEditor(apvts);
    }
}
