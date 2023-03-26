#pragma once

#include <JuceHeader.h>

using namespace juce;

class BaseProcessor : public AudioProcessor
{
// ... generic AudioProcessor setup methods
};

class GainProcessor : public BaseProcessor
{
public:
    GainProcessor(bool c = false)
        : isChild(c)
        , localAvts(*this, nullptr, "PARAMETERS", createProcessorParameters())
    {
        if (!isChild) 
            initializeAvts(localAvts);
    }

    void initializeAvts(AudioProcessorValueTreeState& s)
    {
        avts = &s;
    }
    
    std::unique_ptr<AudioProcessorParameterGroup> createProcessorParameters ()
    {
        std::unique_ptr<AudioProcessorParameterGroup> layout (std::make_unique<AudioProcessorParameterGroup>("groupGain", "Gain", "|"));
        auto gain = std::make_unique<AudioParameterFloat> ("gain", "Gain", 0.0f, 2.0f, 1.0f);
        layout.get()->addChild(std::move(gain));
        return layout;
    }
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        *gainParamValue = avts->getRawParameterValue("gain")->load();
    }
    
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override
    {
        buffer.applyGain(*gainParamValue);
    }

private:
    bool isChild;
    AudioProcessorValueTreeState* avts;
    AudioProcessorValueTreeState localAvts;
    float* gainParamValue;
};

class ParentProcessor : public BaseProcessor {
public:
    //==============================================================================
    ParentProcessor()
        : gainProcessor(true)
        , parentAvts (*this, nullptr, "PARAMETERS", createParameters())
    {
        gainProcessor.initializeAvts(parentAvts);
    }
    
    AudioProcessorValueTreeState::ParameterLayout createParameters()
    {
        std::vector<std::unique_ptr<AudioProcessorParameterGroup>> params;
        params.push_back (gainProcessor.createProcessorParameters());
        return { params.begin(), params.end() };
    }
  
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        gainProcessor.prepareToPlay(sampleRate, samplesPerBlock);
    }

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override
    {
        for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
            buffer.clear (i, 0, buffer.getNumSamples());
        gainProcessor.processBlock(buffer, midiMessages);
    }

    GainProcessor gainProcessor;
    AudioProcessorValueTreeState parentAvts;
};