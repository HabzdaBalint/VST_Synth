/*
==============================================================================

    AdditiveSynthesizer.h
    Created: 5 Mar 2023 15:47:05am
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#ifndef SYNTHPARAMS_INCLUDED
    #include "AdditiveSynthParameters.h"
#endif

class AdditiveSynthesizer : public juce::AudioProcessor
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

    /// @brief Generates a sample of the waveform defined by the parameters of the synthesizer. Used for maintaining the lookup table. This function could also be used for accurate rendering, with any number of harmonics, if time is not a constraint
    /// @param angle The angle at which the sample is generated (in radians)
    /// @param harmonics The number of harmonics that are included in the calculation of the sample. Use lower numbers to avoid aliasing
    /// @return The generated sample
    const float waveTableFormula(float angle, int harmonics);
private:
    std::unique_ptr<AdditiveSynthParameters> synthParameters;

    std::unique_ptr<juce::Synthesiser> synth = std::make_unique<juce::Synthesiser>();
    juce::OwnedArray<juce::dsp::LookupTableTransform<float>> mipMap;
    LUTUpdater lutUpdater { [&] () { updateLookupTable(); } };

    std::unique_ptr<juce::dsp::Gain<float>> synthGain = std::make_unique<juce::dsp::Gain<float>>();

    /// @brief Generates the lookup table with the current parameters
    void updateLookupTable();
    
    const float findGainToNormalize();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthesizer)
};