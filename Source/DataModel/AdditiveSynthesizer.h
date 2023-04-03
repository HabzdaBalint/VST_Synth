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

struct LUTUpdater : juce::Thread
{
    LUTUpdater(std::function<void()> func) :
        juce::Thread("LUT Updater"),
        func(std::move(func))
    {}

    void run() override
    {
        func();
    }
private:
    std::function<void()> func;
};

struct AdditiveSynthParameterReferences;

class AdditiveSynthesizer : public juce::AudioProcessor,
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
    double getTailLengthSeconds() const override { return synthParameters.release/1000; }

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override {};
    void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;

    /// @brief Connects the parameter audio processor value tree state to this processor
    /// @param apvts A reference to an AudioProcessorValueTreeState
    void connectApvts(juce::AudioProcessorValueTreeState& apvts);

    /// @brief Creates and adds the synthesizer's parameters into a parameter group
    /// @return unique pointer to the group
    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout();

    /// @brief Used for making the parameter ids of the the partials' gain parameters consistent
    /// @param index The index of the harmonic
    /// @return A consistent parameter id
    static const juce::String getPartialGainParameterName(size_t index);

    /// @brief Used for making the parameter ids of the the partials' phase parameters consistent
    /// @param index The index of the harmonic
    /// @return A consistent parameter id
    static const juce::String getPartialPhaseParameterName(size_t index);

    /// @brief Generates a sample of the waveform defined by the parameters of the synthesizer. Used for maintaining the lookup table. This function could also be used for accurate rendering, with any number of harmonics, if time is not a constraint
    /// @param angle The angle at which the sample is generated (in radians)
    /// @param harmonics The number of harmonics that are included in the calculation of the sample. Use lower numbers to avoid aliasing
    /// @return The generated sample
    const float waveTableFormula(float angle, int harmonics);

private:
    juce::AudioProcessorValueTreeState* apvts;

    AdditiveSynthParameters synthParameters;
    std::unordered_map<juce::String, std::atomic<float>> paramMap;
    std::unique_ptr<AdditiveSynthParameterReferences> paramRefs;

    std::unique_ptr<juce::Synthesiser> synth = std::make_unique<juce::Synthesiser>();
    juce::OwnedArray<juce::dsp::LookupTableTransform<float>> mipMap;
    LUTUpdater lutUpdater { [&] () { updateLookupTable(); } };

    std::unique_ptr<juce::dsp::Gain<float>> synthGain = std::make_unique<juce::dsp::Gain<float>>();

    void parameterChanged(const juce::String &parameterID, float newValue) override;

    void registerListeners();

    /// @brief Updates atomic parameters
    void updateSynthParameters();

    /// @brief Generates the lookup table with the current parameters
    void updateLookupTable();
    
    const float findGainToNormalize();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthesizer)
};

struct AdditiveSynthParameterReferences
{
    AdditiveSynthParameterReferences(std::unordered_map<juce::String, std::atomic<float>>& paramMap) :
        oscillatorOctaves(&paramMap.at("oscillatorOctaves")),
        oscillatorSemitones(&paramMap.at("oscillatorOctaves")),
        oscillatorFine(&paramMap.at("oscillatorFine")),
        pitchWheelRange(&paramMap.at("pitchWheelRange")),
        globalPhase(&paramMap.at("globalPhase")),
        randomPhaseRange(&paramMap.at("randomPhaseRange")),
        unisonCount(&paramMap.at("unisonCount")),
        unisonDetune(&paramMap.at("unisonDetune")),
        unisonGain(&paramMap.at("unisonGain")),
        amplitudeADSRAttack(&paramMap.at("amplitudeADSRAttack")),
        amplitudeADSRDecay(&paramMap.at("amplitudeADSRDecay")),
        amplitudeADSRSustain(&paramMap.at("amplitudeADSRSustain")),
        amplitudeADSRRelease(&paramMap.at("amplitudeADSRRelease"))
    {
        for (size_t i = 0; i < HARMONIC_N; i++)
        {
            partialGains.push_back(&paramMap.at(AdditiveSynthesizer::getPartialGainParameterName(i)));
            partialPhases.push_back(&paramMap.at(AdditiveSynthesizer::getPartialPhaseParameterName(i)));
        }
    }

    std::vector<std::atomic<float>*> partialGains;
    std::vector<std::atomic<float>*> partialPhases;

    std::atomic<float>* oscillatorOctaves;
    std::atomic<float>* oscillatorSemitones;
    std::atomic<float>* oscillatorFine;
    std::atomic<float>* pitchWheelRange;

    std::atomic<float>* globalPhase;
    std::atomic<float>* randomPhaseRange;

    std::atomic<float>* unisonCount;
    std::atomic<float>* unisonDetune;
    std::atomic<float>* unisonGain;

    std::atomic<float>* amplitudeADSRAttack;
    std::atomic<float>* amplitudeADSRDecay;
    std::atomic<float>* amplitudeADSRSustain;
    std::atomic<float>* amplitudeADSRRelease;
};