/*
==============================================================================

    AdditiveSynthesizer.h
    Created: 5 Mar 2023 15:47:05am
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "AdditiveSynthParameters.h"
#include "AdditiveSound.h"
#include "AdditiveVoice.h"

struct LUTUpdater : juce::Thread
{
    LUTUpdater(std::function<void()> func) :
        juce::Thread("LUT Updater"),
        func(func)
    {}

    void run() override
    {
        func();
    }
private:
    std::function<void()> func;
};

class AdditiveSynthesizer : public juce::AudioProcessor,
                            public juce::AudioProcessorValueTreeState::Listener,
                            public juce::Timer
{
public:
    AdditiveSynthesizer(juce::AudioProcessorValueTreeState&);
    ~AdditiveSynthesizer();

    const juce::String getName() const override { return "Additive Synthesizer"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }

    juce::AudioProcessorEditor* createEditor() override { return nullptr; };
    bool hasEditor() const override { return true; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String &) override {}

    void getStateInformation(juce::MemoryBlock& destData) override {}
    void setStateInformation(const void* data, int sizeInBytes) override {}
    double getTailLengthSeconds() const override { return synthParameters->amplitudeADSRRelease->load() / 1000; }

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override {};
    void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;

    void parameterChanged(const juce::String &parameterID, float newValue) override;

    void timerCallback() override;

    std::unique_ptr<AdditiveSynthParameters> synthParameters;
    std::unique_ptr<OscillatorParameters> oscParameters;
private:
    std::unique_ptr<juce::Synthesiser> synth = std::make_unique<juce::Synthesiser>();
    juce::OwnedArray<juce::dsp::LookupTableTransform<float>> mipMap;
    LUTUpdater lutUpdater { [&] () { updateLookupTable(); } };
    std::atomic<bool> missedUpdate = { false };

    std::unique_ptr<juce::dsp::Gain<float>> synthGain = std::make_unique<juce::dsp::Gain<float>>();

    /// @brief Generates the lookup table with the current parameters
    void updateLookupTable();
    
    const float getPeakAmplitude();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthesizer)
};