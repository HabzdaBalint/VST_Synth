/*
  ==============================================================================

    AdditiveSynthesizer.h
    Created: 5 Mar 2023 15:47:05am
    Author:  Habama10

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AdditiveSound.h"
#include "AdditiveVoice.h"
#include "SynthParameters.h"

class AdditiveSynthesizer : public juce::AudioProcessor
{
public:
    AdditiveSynthesizer() : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo()))
    {
        for (size_t i = 0; i < 8; i++)
        {
            mipMap.add(new juce::dsp::LookupTableTransform<float>());
        }

        synth.addSound(new AdditiveSound());

        for (size_t i = 0; i < SYNTH_MAX_VOICES; i++)
        {
            synth.addVoice(new AdditiveVoice(synthParameters, mipMap, 8));
        }
        synth.setNoteStealingEnabled(true);
    }

    ~AdditiveSynthesizer() 
    {
        for (size_t i = 0; i < mipMap.size(); i++)
        {
            delete(mipMap[i]);
        }
        mipMap.removeRange(0, mipMap.size());
    }

    const juce::String getName() const override { return "Additive Synth"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
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

    void releaseResources() override {}

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override
    {
        synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

        juce::dsp::AudioBlock<float> audioBlock{ buffer };
        synthGain.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
    }

    double getTailLengthSeconds() const override { return 0;}
    
    void getStateInformation(juce::MemoryBlock& destData) {}

    void setStateInformation(const void* data, int sizeInBytes) {}

    void updateParameters(juce::AudioProcessorValueTreeState& apvts)
    {
        synthGain.setGainDecibels(apvts.getRawParameterValue("synthGain")->load());
        synthParameters.octaveTuning = apvts.getRawParameterValue("oscillatorOctaves")->load();
        synthParameters.semitoneTuning = apvts.getRawParameterValue("oscillatorSemitones")->load();
        synthParameters.fineTuningCents = apvts.getRawParameterValue("oscillatorFine")->load();

        synthParameters.globalPhseStart = apvts.getRawParameterValue("globalPhase")->load();
        synthParameters.randomPhaseRange = apvts.getRawParameterValue("globalPhaseRNG")->load();

        synthParameters.unisonPairCount = apvts.getRawParameterValue("unisonCount")->load();
        synthParameters.unisonGain = apvts.getRawParameterValue("unisonGain")->load();
        synthParameters.unisonDetune = apvts.getRawParameterValue("unisonDetune")->load();

        juce::String paramId;
        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            paramId << "partial" << i + 1 << "Gain";
            synthParameters.partialGain[i] = apvts.getRawParameterValue(paramId)->load();
            paramId.clear();
            paramId << "partial" << i + 1 << "Phase";
            synthParameters.partialPhase[i] = apvts.getRawParameterValue(paramId)->load();
            paramId.clear();
        }

        updateLookupTable();
    }

    void updateLookupTable()
    {
        for (size_t i = 0; i < 8; i++)
        {
            mipMap[i]->initialise([this, i](float x) { return WaveTableFormula(x, HARMONIC_N / pow(2, i)); }, 0, juce::MathConstants<float>::twoPi, 1024);
        }
    }

private:
    juce::Synthesiser synth;
    juce::dsp::Gain<float> synthGain;

    SynthParameters synthParameters; 

    juce::Array<juce::dsp::LookupTableTransform<float>*> mipMap;

    /*Generates the full formula for the current setup of the additive synth. Used for maintaining the lookup table. This function could also be used for accurate rendering, if time is not a constraint*/
    const float WaveTableFormula(float angle, float harmonics)
    {
        float sample = 0.f;
        
        /*Generating a single sample with every harmonic.*/
        for (size_t i = 0; i < harmonics; i++)
        {
            if (synthParameters.partialGain[i] != 0.f)
            {
                sample += synthParameters.partialGain[i] * sin((i + 1) * angle + synthParameters.partialPhase[i] * juce::MathConstants<float>::pi);
            }
        }
        return sample;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthesizer)
};
