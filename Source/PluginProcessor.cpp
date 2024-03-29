/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

VST_SynthAudioProcessor::VST_SynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                    #if ! JucePlugin_IsMidiEffect
                    #if ! JucePlugin_IsSynth
                    .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                    #endif
                    .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                    #endif
                    )
#endif
{}

VST_SynthAudioProcessor::~VST_SynthAudioProcessor() {}

const juce::String VST_SynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VST_SynthAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool VST_SynthAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool VST_SynthAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double VST_SynthAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int VST_SynthAudioProcessor::getNumPrograms() { return 1; }

int VST_SynthAudioProcessor::getCurrentProgram() { return 0; }

void VST_SynthAudioProcessor::setCurrentProgram (int index) {}

const juce::String VST_SynthAudioProcessor::getProgramName (int index) { return "None"; }

void VST_SynthAudioProcessor::changeProgramName (int index, const juce::String& newName) { }

void VST_SynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    additiveSynth.prepareToPlay(sampleRate, samplesPerBlock);
    fxChain.prepareToPlay(sampleRate, samplesPerBlock);

    for(int i = 0; i < 2; i++)
    {
        synthRMS[i].reset(sampleRate, 0.3);
        synthRMS[i].setCurrentAndTargetValue(-90.f);
        atomicSynthRMS[i].set(-90.f);
    }
}

void VST_SynthAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VST_SynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
    && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    return true;
#endif
}
#endif

void VST_SynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();

    keyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true);

    for(int i = totalNumInputChannels; i < totalNumOutputChannels; i++)
        buffer.clear (i, 0, numSamples);

    additiveSynth.processBlock(buffer, midiMessages);

    for(int i = 0; i < 2; i++)
    {
        synthRMS[i].skip(numSamples);
        auto value = juce::Decibels::gainToDecibels(buffer.getRMSLevel(i, 0, numSamples));
        if(value > synthRMS[i].getCurrentValue())
        {
            synthRMS[i].setCurrentAndTargetValue(value);
        }
        else
        {
            synthRMS[i].setTargetValue(value);
        }
        atomicSynthRMS[i].set(synthRMS[i].getCurrentValue());
    }

    fxChain.processBlock(buffer, midiMessages);
    
    midiMessages.clear();
}

bool VST_SynthAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* VST_SynthAudioProcessor::createEditor()
{
    return new Editor::VST_SynthAudioProcessorEditor(*this);
}

void VST_SynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream outputStream(destData, true);
    apvts.state.writeToStream(outputStream);
}

void VST_SynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout VST_SynthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> layout;

    using namespace Processor::Synthesizer;
    layout.push_back(OscillatorParameters::createParameterLayout());
    layout.push_back(AdditiveSynthParameters::createParameterLayout());

    using namespace Processor::Effects;
    layout.push_back(EffectsChain::createParameterLayout());
    
    layout.push_back(Equalizer::createParameterLayout());
    layout.push_back(Filter::createParameterLayout());
    layout.push_back(Compressor::createParameterLayout());
    layout.push_back(Delay::createParameterLayout());
    layout.push_back(Reverb::createParameterLayout());
    layout.push_back(Chorus::createParameterLayout());
    layout.push_back(Phaser::createParameterLayout());
    layout.push_back(Tremolo::createParameterLayout());

    return { layout.begin(), layout.end() };
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VST_SynthAudioProcessor();
}
