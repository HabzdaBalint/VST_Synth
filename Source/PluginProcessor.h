/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Model/Synthesizer/AdditiveSynthesizer.h"
#include "Model/Effects/EffectProcessorChain.h"

class VST_SynthAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
    , public juce::AudioProcessorARAExtension
#endif
{
public:
    //==============================================================================
    VST_SynthAudioProcessor();
    ~VST_SynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, juce::Identifier(JucePlugin_Name), createParameterLayout() };

    Processor::Synthesizer::AdditiveSynthesizer additiveSynth = Processor::Synthesizer::AdditiveSynthesizer(apvts);
    Processor::Effects::EffectsChain::EffectProcessorChain fxChain = Processor::Effects::EffectsChain::EffectProcessorChain(apvts);

    juce::LinearSmoothedValue<float> synthRMS[2];
    juce::Atomic<float> atomicSynthRMS[2];

    juce::MidiKeyboardState keyboardState;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST_SynthAudioProcessor)
};