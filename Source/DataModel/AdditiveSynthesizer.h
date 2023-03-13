/*
  ==============================================================================

    AdditiveSynthesizer.h
    Created: 5 Mar 2023 15:47:05am
    Author:  Habama10

  ==============================================================================
*/

#pragma once

#include "SynthParameters.h"
#include "AdditiveSound.h"
#include "AdditiveVoice.h"

class AdditiveSynthesizer : public juce::AudioProcessor
{
public:
    AdditiveSynthesizer() : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo()))
    {
        for (size_t i = 0; i < LOOKUP_SIZE; i++)
        {
            mipMap.add(new juce::dsp::LookupTableTransform<float>());
        }

        synth.addSound(new AdditiveSound());

        for (size_t i = 0; i < SYNTH_MAX_VOICES; i++)
        {
            synth.addVoice(new AdditiveVoice(synthParameters, mipMap));
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

    const juce::String getName() const override { return "Additive Synthesizer"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    juce::AudioProcessorEditor* createEditor() override { return nullptr; } //todo return the synth's editor object
    bool hasEditor() const override { return true; }

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        synth.setCurrentPlaybackSampleRate(sampleRate);
        for (size_t i = 0; i < synth.getNumVoices(); i++)
        {
            if (auto voice = dynamic_cast<AdditiveVoice*>(synth.getVoice(i)))
            {
                voice->setCurrentPlaybackSampleRate(sampleRate);
                voice->amplitudeADSR.setSampleRate(sampleRate);
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
    
    void getStateInformation(juce::MemoryBlock& destData) override {}

    void setStateInformation(const void* data, int sizeInBytes) override {}

    void updateParameters(juce::AudioProcessorValueTreeState& apvts)
    {
        synthGain.setGainDecibels(apvts.getRawParameterValue("synthGain")->load());
        synthParameters.octaveTuning = apvts.getRawParameterValue("oscillatorOctaves")->load();
        synthParameters.semitoneTuning = apvts.getRawParameterValue("oscillatorSemitones")->load();
        synthParameters.fineTuningCents = apvts.getRawParameterValue("oscillatorFine")->load();

        synthParameters.pitchWheelRange = apvts.getRawParameterValue("pitchWheelRange")->load();

        synthParameters.globalPhseStart.phase = apvts.getRawParameterValue("globalPhase")->load();
        synthParameters.randomPhaseRange.phase = apvts.getRawParameterValue("globalPhaseRNG")->load();

        synthParameters.unisonPairCount = apvts.getRawParameterValue("unisonCount")->load();
        synthParameters.unisonGain = apvts.getRawParameterValue("unisonGain")->load();
        synthParameters.unisonDetune = apvts.getRawParameterValue("unisonDetune")->load();

        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            synthParameters.partialGain[i] = apvts.getRawParameterValue(getPartialGainParameterName(i))->load();
            synthParameters.partialPhase[i].phase = apvts.getRawParameterValue(getPartialPhaseParameterName(i))->load();
        }

        /*todo optimize*/
        updateLookupTable();

        synthParameters.amplitudeADSRParams.attack = apvts.getRawParameterValue("amplitudeADSRAttack")->load()/1000;
        synthParameters.amplitudeADSRParams.decay = apvts.getRawParameterValue("amplitudeADSRDecay")->load()/1000;
        synthParameters.amplitudeADSRParams.sustain = apvts.getRawParameterValue("amplitudeADSRSustain")->load();
        synthParameters.amplitudeADSRParams.release = apvts.getRawParameterValue("amplitudeADSRRelease")->load()/1000;

        for (size_t i = 0; i < synth.getNumVoices(); i++)
        {
            if (auto voice = dynamic_cast<AdditiveVoice*>(synth.getVoice(i)))
            {
                voice->amplitudeADSR.setParameters(synthParameters.amplitudeADSRParams);
            }
        }
    }

    void updateLookupTable()
    {
        for (size_t i = 0; i < LOOKUP_SIZE; i++)
        {
            mipMap[i]->initialise([this, i](float x) { return WaveTableFormula(x, HARMONIC_N / pow(2, i)); }, 0, juce::MathConstants<float>::twoPi, 1024);
        }
    }

    void createParameterLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
    {
        //---------------------// SYNTH PARAMS //---------------------//
        /*Master Gain for the oscillator. All voices are affected by this value*/
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "synthGain",
            "Gain",
            juce::NormalisableRange<float>(-90.f, 0.f, 0.1),-12.0f));

        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            juce::String namePrefix = "Partial " + juce::String(i+1) + " ";
            //Generating parameters to represent the linear gain values of the partials
            layout.add(std::make_unique <juce::AudioParameterFloat>(
                getPartialGainParameterName(i), 
                namePrefix + "Gain", 
                juce::NormalisableRange<float>(0.f, 1.f, 0.001), 0.f));

            //Generating parameters to represent the phase of the partials. These are represented as multiples of 2*pi
            layout.add(std::make_unique <juce::AudioParameterFloat>(
                getPartialPhaseParameterName(i), 
                namePrefix + "Phase", 
                juce::NormalisableRange<float>(0.f, 1.f, 0.01), 0.f));
        }

        /*Tuning of the generated notes in octaves*/
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "oscillatorOctaves",
            "Octaves",
            juce::NormalisableRange<float>(-2, 2, 1),
            0));

        /*Tuning of the generated notes in semitones*/
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "oscillatorSemitones",
            "Semitones",
            juce::NormalisableRange<float>(-12, 12, 1),
            0));

        /*Tuning of the generated notes in cents*/
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "oscillatorFine",
            "Fine Tuning",
            juce::NormalisableRange<float>(-100, 100, 1),
            0));

        /*Pitch Wheel range in semitones*/
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "pitchWheelRange",
            "Pitch Wheel Semitones",
            juce::NormalisableRange<float>(0, 12, 1),
            2));

        /*The global starting point of on waveform. in multiples of 2*pi*/
        layout.add(std::make_unique <juce::AudioParameterFloat>(
            "globalPhase",
            "Phase",
            juce::NormalisableRange<float>(0.f, 1.f, 0.01),
            0.f));

        /*Sets the angle range for phase start randomization on new voices and unison. multiple of 2*pi*/
        layout.add(std::make_unique <juce::AudioParameterFloat>(
            "globalPhaseRNG",
            "Phase Randomness",
            juce::NormalisableRange<float>(0.f, 1.f, 0.01),
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

        /*Level of the unison in linear amplitude*/
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "unisonGain",
            "Unison Gain",
            juce::NormalisableRange<float>(0.f, 1.f, 0.001),
            0.f));

        /*Attack time for the oscillator's amplitudes in ms*/
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "amplitudeADSRAttack",
            "A",
            juce::NormalisableRange<float>(0.f, 16000.f, 0.1),
            0.5));

        /*Decay time for the oscillator's amplitudes in ms*/
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "amplitudeADSRDecay",
            "D",
            juce::NormalisableRange<float>(0.f, 16000.f, 0.1),
            1000.f));

        /*Sustain level for the oscillator's amplitudes in linear amplitude*/
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "amplitudeADSRSustain",
            "S",
            juce::NormalisableRange<float>(0.f, 1.f, 0.001),
            1.f));

        /*Release time for the oscillator's amplitudes in ms*/
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "amplitudeADSRRelease",
            "R",
            juce::NormalisableRange<float>(0.f, 16000.f, 0.1),
            50.f));
    }

    juce::String getPartialGainParameterName(size_t index)
    {
        return "partial" + juce::String(index) + "gain";
    }

    juce::String getPartialPhaseParameterName(size_t index)
    {
        return "partial" + juce::String(index) + "phase";
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
        
        /*Generating a single sample using every harmonic.*/
        for (size_t i = 0; i < harmonics; i++)
        {
            if (synthParameters.partialGain[i] != 0.f)
            {
                sample += synthParameters.partialGain[i] * sin((i + 1) * angle + synthParameters.partialPhase[i].phase * juce::MathConstants<float>::twoPi);
            }
        }
        return sample;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthesizer)
};
