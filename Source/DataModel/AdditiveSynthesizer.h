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

class AdditiveSynthesizer : public juce::AudioProcessor
{
public:
    AdditiveSynthesizer();
    ~AdditiveSynthesizer();

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    juce::AudioProcessorEditor* createEditor();
    bool hasEditor() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String &) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    double getTailLengthSeconds() const override;

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;

    void registerListeners(juce::AudioProcessorValueTreeState&);

    /// @brief Generates a sample of the waveform defined by the parameters of the synthesizer. Used for maintaining the lookup table. This function could also be used for accurate rendering, with any number of harmonics, if time is not a constraint
    /// @param angle The angle at which the sample is generated (in radians)
    /// @param harmonics The number of harmonics that are included in the calculation of the sample. Use lower numbers to avoid aliasing
    /// @return The generated sample
    const float WaveTableFormula(float angle, int harmonics);

    AdditiveSynthParameters synthParameters{ [this] () { updateSynthParameters(); } };
private:
    juce::Synthesiser* synth = new juce::Synthesiser();
    juce::dsp::Gain<float>* synthGain = new juce::dsp::Gain<float>();
    AdditiveSynthParametersAtomic synthParametersAtomic;

    juce::Array<juce::dsp::LookupTableTransform<float> *> mipMap;

    /// @brief Updates atomic parameters
    void updateSynthParameters();
    /// @brief Generates the lookup table with the current parameters
    void updateLookupTable();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthesizer)
};
