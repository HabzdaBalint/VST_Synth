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
}

VST_SynthAudioProcessor::~VST_SynthAudioProcessor()
{
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

double VST_SynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VST_SynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VST_SynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VST_SynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VST_SynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void VST_SynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VST_SynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    updateParameters();
    additiveSynth.setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);
    additiveSynth.prepareToPlay(sampleRate, samplesPerBlock);
}

void VST_SynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VST_SynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
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
    updateParameters();

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    //auto* channelData = buffer.getWritePointer(0);
    //auto* channelData = buffer.getWritePointer(1);

    additiveSynth.processBlock(buffer, midiMessages);

    /*todo other fx*/
}

//==============================================================================
bool VST_SynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VST_SynthAudioProcessor::createEditor()
{
    //return new VST_SynthAudioProcessorEditor (*this);
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
        updateParameters();
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout VST_SynthAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    //---------------------//SYNTH PARAMS//---------------------//
    /*Master Gain for the oscillator. All voices are affected by this value*/
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "synthGain",
        "Gain",
        juce::NormalisableRange<float>(-90.f, 0.f, 0.1),
        -6.0f));

    juce::String paramId;
    juce::String paramName;
    for (size_t i = 0; i < HARMONIC_N; i++)
    {
        //Generating parameters to represent the linear gain values of the 100 partials
        paramId << "partial" << i + 1 << "Gain";
        paramName << "Partial #" << i + 1 << " Gain";
        layout.add(std::make_unique <juce::AudioParameterFloat>(paramId, paramName, juce::NormalisableRange<float>(0.f, 1.f, 0.01), 0.f));
        paramId.clear();
        paramName.clear();

        //Generating parameters to represent the phase of the 100 partials. These are represented as multiples of pi
        paramId << "partial" << i + 1 << "Phase";
        paramName << "Partial #" << i + 1 << " Phase";
        layout.add(std::make_unique <juce::AudioParameterFloat>(paramId, paramName, juce::NormalisableRange<float>(0.f, 2.f, 0.01), 0.f));
        paramId.clear();
        paramName.clear();
    }

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "oscillatorOctaves",
        "Octaves",
        juce::NormalisableRange<float>(-2, 2, 1),
        0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "oscillatorSemitones",
        "Semitones",
        juce::NormalisableRange<float>(-12, 12, 1),
        0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "oscillatorFine",
        "Fine Tuning",
        juce::NormalisableRange<float>(-100, 100, 1),
        0));


    /*The global starting point of on waveform*/
    layout.add(std::make_unique <juce::AudioParameterFloat>(
        "globalPhase",
        "Phase",
        juce::NormalisableRange<float>(0.f, 2.f, 0.01),
        0.f));

    /*Sets the angle range for phase start randomization on new voices and unison*/
    layout.add(std::make_unique <juce::AudioParameterFloat>(
        "globalPhaseRNG",
        "Phase Randomness",
        juce::NormalisableRange<float>(0.f, 1.f, 0.1),
        0));

    /*Pairs of unison to add (one tuned higher and one lower)*/
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "unisonCount",
        "Unison Count",
        juce::NormalisableRange<float>(0, 5, 1),
        0));

    /*Detuning of the farthest unison pair in cents. The pairs inbetween have a tuning that is evenly distributed between the normal frequency and this one*/
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "unisonDetune",
        "Unison Detune",
        juce::NormalisableRange<float>(0, 100, 1),
        0));

    /*Linear gain value of the unison (always added over fundamental)*/
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "unisonGain",
        "Unison Gain",
        juce::NormalisableRange<float>(0.f, 1.f, 0.01),
        0.f));

    return layout;
}

void VST_SynthAudioProcessor::updateParameters()
{
    additiveSynth.updateParameters(apvts);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VST_SynthAudioProcessor();
}
