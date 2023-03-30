/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
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
{
    additiveSynth->connectApvts(apvts);
    fxChain->connectApvts(apvts);
}

VST_SynthAudioProcessor::~VST_SynthAudioProcessor()
{
    delete(additiveSynth);
    delete(fxChain);
}

//==============================================================================
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

const juce::String VST_SynthAudioProcessor::getProgramName (int index) { return {}; }

void VST_SynthAudioProcessor::changeProgramName (int index, const juce::String& newName) { }

//==============================================================================
void VST_SynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    additiveSynth->prepareToPlay(sampleRate, samplesPerBlock);
    fxChain->setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);
    fxChain->prepareToPlay(sampleRate, samplesPerBlock);
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

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    additiveSynth->processBlock(buffer, midiMessages);

    fxChain->processBlock(buffer, midiMessages);
}

//==============================================================================
bool VST_SynthAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* VST_SynthAudioProcessor::createEditor()
{
    //return new VST_SynthAudioProcessorEditor(*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
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

    layout.push_back(additiveSynth->createParameterLayout());

    fxChain->createParameters(layout);

    return { layout.begin(), layout.end() };
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VST_SynthAudioProcessor();
}
