/*
==============================================================================

    FXProcessorUnit.h
    Created: 13 Mar 2023 2:15:01pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class FXProcessorUnit : public juce::AudioProcessor,
                        public juce::AudioProcessorValueTreeState::Listener
{
public:
    FXProcessorUnit(juce::AudioProcessorValueTreeState& apvts) : apvts(apvts),
                    AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo())
                                                    .withOutput("Output", juce::AudioChannelSet::stereo()))
    {}

    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    void processBlock(juce::AudioSampleBuffer&, juce::MidiBuffer&) override {}

    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return {}; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    double getTailLengthSeconds() const override { return 0; }

    int getNumPrograms() override { return 0; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    virtual void registerListeners() = 0;

protected:
    juce::AudioProcessorValueTreeState& apvts;    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXProcessorUnit)
};