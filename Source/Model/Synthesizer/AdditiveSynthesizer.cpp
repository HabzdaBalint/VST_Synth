/*
==============================================================================

    AdditiveSynthesizer.cpp
    Created: 25 Mar 2023 9:23:43pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "AdditiveSynthesizer.h"

namespace Processor::Synthesizer
{
    AdditiveSynthesizer::AdditiveSynthesizer(juce::AudioProcessorValueTreeState& apvts) :
                            AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo())),
                            oscParameters(apvts),
                            synthParameters(apvts)
    {
        synth.addSound(new AdditiveSound());

        for (size_t i = 0; i < SYNTH_MAX_VOICES; i++)
        {
            synth.addVoice(new AdditiveVoice(synthParameters, oscParameters.getLookupTable()));
        }
        synth.setNoteStealingEnabled(true);
    }

    AdditiveSynthesizer::~AdditiveSynthesizer()
    {
        synth.clearVoices();
        synth.clearSounds();
    }

    void AdditiveSynthesizer::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock)
    {
        synth.setCurrentPlaybackSampleRate(sampleRate);
        for (size_t i = 0; i < synth.getNumVoices(); i++)
        {
            if (auto voice = dynamic_cast<AdditiveVoice*>(synth.getVoice(i)))
            {
                voice->setCurrentPlaybackSampleRate(sampleRate);
            }
        }

        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
        processSpec.numChannels = getTotalNumOutputChannels();
        processSpec.sampleRate = sampleRate;
        synthGain.prepare(processSpec);
    }

    void AdditiveSynthesizer::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
    {
        synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

        juce::dsp::AudioBlock<float> audioBlock { buffer };

        synthGain.setGainLinear(synthParameters.synthGain->load() / 100);

        synthGain.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
    }
}
