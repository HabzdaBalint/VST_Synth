/*
==============================================================================

    FXProcessorChainParameters.h
    Created: 12 Mar 2023 07:32:57pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FXEqualizer.h"
#include "FXFilter.h"
#include "FXCompressor.h"
#include "FXDelay.h"
#include "FXReverb.h"
#include "FXChorus.h"
#include "FXPhaser.h"
#include "FXTremolo.h"

constexpr int FX_MAX_SLOTS = 8;//todo probably remove

class FXProcessorChain : public juce::AudioProcessor,
                         juce::AudioProcessorValueTreeState::Listener
{
public:
    FXProcessorChain();

    ~FXProcessorChain();

    const juce::String getName() const override { return "Effect Chain"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    double getTailLengthSeconds() const override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioSampleBuffer &buffer, juce::MidiBuffer &midiMessages) override;

    void connectApvts(juce::AudioProcessorValueTreeState& apvts);

    void registerListeners();
    void createParameters(std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> &layout);

    std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout();

    /// @brief Used for making the parameter ids of the the FX slots' bypass parameters consistent
    /// @param index The index of the effect
    /// @return A consistent parameter id
    juce::String getFXBypassParameterName(size_t index)
    {
        return "bypass" + juce::String(index);
    }

    /// @brief Used for making the parameter ids of the the FX slots' choice parameters consistent
    /// @param index The index of the effect
    /// @return A consistent parameter id
    juce::String getFXChoiceParameterName(size_t index)
    {
        return "fxSlot" + juce::String(index);
    }

    //todo: only have one owned array of unique processor objects instead of an instance of all of them. make it modifiable and organizable with functions
    juce::OwnedArray<juce::AudioProcessor> processors;

    FXEqualizer* equalizer = new FXEqualizer();
    FXFilter* filter = new FXFilter();
    FXCompressor* compressor = new FXCompressor();
    FXDelay* delay = new FXDelay();
    FXReverb* reverb = new FXReverb();
    FXChorus* chorus = new FXChorus();
    FXPhaser* phaser = new FXPhaser();
    FXTremolo* tremolo = new FXTremolo();
private:
    juce::AudioProcessorValueTreeState* apvts;

    const juce::StringArray choices = {"Empty", "EQ", "Fliter", "Compressor", "Delay", "Reverb", "Chorus", "Phaser", "Tremolo"};

    juce::AudioProcessor* fxProcessorChain[FX_MAX_SLOTS] = {};
    bool bypassSlot[FX_MAX_SLOTS] = {};

    void parameterChanged(const juce::String &parameterID, float newValue) override;

    void updateGraph();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXProcessorChain)
};