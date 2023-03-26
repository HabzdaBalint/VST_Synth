/*
==============================================================================

    FXProcessorChainParameters.h
    Created: 12 Mar 2023 07:32:57pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FXProcessorChainParameters.h"
#include "FXEqualizer.h"
#include "FXFilter.h"
#include "FXCompressor.h"
#include "FXDelay.h"
#include "FXReverb.h"
#include "FXChorus.h"
#include "FXPhaser.h"
#include "FXTremolo.h"

class FXProcessorChain : public juce::AudioProcessor
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

    FXProcessorChainParameters chainParameters{ [this] () { updateGraph(); } };

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
    juce::AudioProcessorValueTreeState localapvts = { *this, nullptr, "Effects Chain Parameters", chainParameters.createParameterLayout() };;

    juce::AudioProcessor* fxProcessorChain[FX_MAX_SLOTS] = {};
    bool bypassSlot[FX_MAX_SLOTS] = {};

    void updateGraph();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXProcessorChain)
};