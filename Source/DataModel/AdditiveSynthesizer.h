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

class AdditiveSynthesizer : public juce::AudioProcessor,
                            juce::AsyncUpdater,
                            juce::AudioProcessorValueTreeState::Listener
{
public:
    AdditiveSynthesizer();
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
    double getTailLengthSeconds() const override { return 0; }

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;

    /// @brief Connects the parameter audio processor value tree state to this processor
    /// @param apvts A reference to an AudioProcessorValueTreeState
    void connectApvts(juce::AudioProcessorValueTreeState& apvts);

    /// @brief Creates and adds the synthesizer's parameters into a parameter group
    /// @return unique pointer to the group
    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout();

    /// @brief Generates a sample of the waveform defined by the parameters of the synthesizer. Used for maintaining the lookup table. This function could also be used for accurate rendering, with any number of harmonics, if time is not a constraint
    /// @param angle The angle at which the sample is generated (in radians)
    /// @param harmonics The number of harmonics that are included in the calculation of the sample. Use lower numbers to avoid aliasing
    /// @return The generated sample
    const float WaveTableFormula(float angle, int harmonics);

    /// @brief Used for making the parameter ids of the the partials' gain parameters consistent
    /// @param index The index of the harmonic
    /// @return A consistent parameter id
    juce::String getPartialGainParameterName(size_t index);

    /// @brief Used for making the parameter ids of the the partials' phase parameters consistent
    /// @param index The index of the harmonic
    /// @return A consistent parameter id
    juce::String getPartialPhaseParameterName(size_t index);

private:
    juce::AudioProcessorValueTreeState* apvts;
    juce::AudioProcessorValueTreeState localapvts = { *this, nullptr, "Synthesizer Parameters", createParameterLayout() };

    juce::Synthesiser* synth = new juce::Synthesiser();
    juce::dsp::Gain<float>* synthGain = new juce::dsp::Gain<float>();
    AdditiveSynthParametersAtomic synthParametersAtomic;

    juce::Array<juce::dsp::LookupTableTransform<float> *> mipMap;

    void parameterChanged(const juce::String &parameterID, float newValue) override;

    void registerListeners();

    void handleAsyncUpdate() override;

    /// @brief Updates atomic parameters
    void updateSynthParameters();

    /// @brief Generates the lookup table with the current parameters
    void updateLookupTable();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthesizer)
};
