/*
==============================================================================

    AdditiveSynthesizer.h
    Created: 5 Mar 2023 15:47:05am
    Author:  Habama10

==============================================================================
*/

#pragma once

#include "AdditiveSynthParameters.h"
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

        synth->addSound(new AdditiveSound());

        for (size_t i = 0; i < SYNTH_MAX_VOICES; i++)
        {
            synth->addVoice(new AdditiveVoice(synthParametersAtomic, mipMap));
        }
        synth->setNoteStealingEnabled(true);
    }

    ~AdditiveSynthesizer()
    {
        for (size_t i = 0; i < mipMap.size(); i++)
        {
            delete (mipMap[i]);
        }
        mipMap.removeRange(0, mipMap.size());
        delete(synthGain);
        delete(synth);
    }

    const juce::String getName() const override { return "Additive Synthesizer"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }

    juce::AudioProcessorEditor* createEditor() override { return nullptr; } // todo return the synth's editor object
    bool hasEditor() const override { return true; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String &) override {}

    void getStateInformation(juce::MemoryBlock& destData) override {}
    void setStateInformation(const void* data, int sizeInBytes) override {}

    double getTailLengthSeconds() const override { return 0; }

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        synth->setCurrentPlaybackSampleRate(sampleRate);
        for (size_t i = 0; i < synth->getNumVoices(); i++)
        {
            if (auto voice = dynamic_cast<AdditiveVoice *>(synth->getVoice(i)))
            {
                voice->setCurrentPlaybackSampleRate(sampleRate);
                voice->amplitudeADSR.setSampleRate(sampleRate);
            }
        }

        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = maximumExpectedSamplesPerBlock;
        processSpec.numChannels = getTotalNumOutputChannels();
        processSpec.sampleRate = sampleRate;
        synthGain->prepare(processSpec);
    }

    void releaseResources() override {}

    void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override
    {
        synth->renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

        juce::dsp::AudioBlock<float> audioBlock{buffer};
        synthGain->process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
    }

    void updateSynthParameters()
    {
        synthGain->setGainDecibels(synthParameters.synthGain->get());
        synthParametersAtomic.octaveTuning = synthParameters.octaveTuning->get();
        synthParametersAtomic.semitoneTuning = synthParameters.semitoneTuning->get();
        synthParametersAtomic.fineTuningCents = synthParameters.fineTuningCents->get();

        synthParametersAtomic.pitchWheelRange = synthParameters.pitchWheelRange->get();

        synthParametersAtomic.globalPhseStart = synthParameters.globalPhseStart->get();
        synthParametersAtomic.randomPhaseRange = synthParameters.randomPhaseRange->get();

        synthParametersAtomic.unisonPairCount = synthParameters.unisonPairCount->get();
        synthParametersAtomic.unisonGain = synthParameters.unisonGain->get();
        synthParametersAtomic.unisonDetune = synthParameters.unisonDetune->get();

        synthParametersAtomic.attack = synthParameters.attack->get() / 1000;
        synthParametersAtomic.decay = synthParameters.decay->get() / 1000;
        synthParametersAtomic.sustain = synthParameters.sustain->get();
        synthParametersAtomic.release = synthParameters.release->get() / 1000;

        juce::String paramId;
        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            synthParametersAtomic.partialGain[i] = synthParameters.partialGain[i]->get();
            synthParametersAtomic.partialPhase[i] = synthParameters.partialPhase[i]->get();
        }

        updateLookupTable();
    }

    void updateLookupTable()
    {
        for (size_t i = 0; i < LOOKUP_SIZE; i++)
        {
            mipMap[i]->initialise([this, i](float x)
                                  { return WaveTableFormula(x, HARMONIC_N / pow(2, i)); },
                                  0, juce::MathConstants<float>::twoPi, 2048);
        }
    }

    void registerListeners(juce::AudioProcessorValueTreeState &apvts)
    {
        synthParameters.update();

        apvts.addParameterListener("synthGain", &synthParameters);

        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            apvts.addParameterListener(synthParameters.getPartialGainParameterName(i), &synthParameters);
            apvts.addParameterListener(synthParameters.getPartialPhaseParameterName(i), &synthParameters);
        }
        apvts.addParameterListener("oscillatorOctaves", &synthParameters);
        apvts.addParameterListener("oscillatorSemitones", &synthParameters);
        apvts.addParameterListener("oscillatorFine", &synthParameters);
        apvts.addParameterListener("pitchWheelRange", &synthParameters);
        apvts.addParameterListener("globalPhase", &synthParameters);
        apvts.addParameterListener("globalPhaseRNG", &synthParameters);
        apvts.addParameterListener("unisonCount", &synthParameters);
        apvts.addParameterListener("unisonDetune", &synthParameters);
        apvts.addParameterListener("unisonGain", &synthParameters);
        apvts.addParameterListener("amplitudeADSRAttack", &synthParameters);
        apvts.addParameterListener("amplitudeADSRDecay", &synthParameters);
        apvts.addParameterListener("amplitudeADSRSustain", &synthParameters);
        apvts.addParameterListener("amplitudeADSRRelease", &synthParameters);
    }

    AdditiveSynthParameters synthParameters{[this]()
                                    { updateSynthParameters(); }};
private:
    juce::Synthesiser* synth = new juce::Synthesiser();
    juce::dsp::Gain<float>* synthGain = new juce::dsp::Gain<float>();
    AdditiveSynthParametersAtomic synthParametersAtomic;

    juce::Array<juce::dsp::LookupTableTransform<float> *> mipMap;

    /*Generates the full formula for the current setup of the additive synth. Used for maintaining the lookup table. This function could also be used for accurate rendering, if time is not a constraint*/
    const float WaveTableFormula(float angle, float harmonics)
    {
        float sample = 0.f;

        /*Generating a single sample using every harmonic.*/
        for (size_t i = 0; i < harmonics; i++)
        {
            if (synthParametersAtomic.partialGain[i] != 0.f)
            {
                sample += synthParametersAtomic.partialGain[i] * sin((i + 1) * angle + synthParametersAtomic.partialPhase[i] * juce::MathConstants<float>::twoPi);
            }
        }
        return sample;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthesizer)
};
