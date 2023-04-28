/*
==============================================================================

    TremoloProcessor.cpp
    Created: 11 Apr 2023 9:39:14pm
    Author:  Habama10

==============================================================================
*/

#include "TremoloProcessor.h"

#include "../../../View/Effects/Tremolo/TremoloEditor.h"

namespace Effects::Tremolo
{
    TremoloProcessor::TremoloProcessor(juce::AudioProcessorValueTreeState& apvts) : EffectProcessor(apvts)
    {
        registerListener(this);
    }

    TremoloProcessor::~TremoloProcessor() 
    {
        removeListener(this);
    }

    void TremoloProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) 
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);

        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = samplesPerBlock;
        processSpec.numChannels = getMainBusNumOutputChannels();
        processSpec.sampleRate = sampleRate;
        updateTremoloParameters();
    }

    void TremoloProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) 
    {
        juce::dsp::AudioBlock<float> audioBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> context(audioBlock);

        float amplitudeMultiplier = 0;

        auto *leftBufferPointer = buffer.getWritePointer(0);
        auto *rightBufferPointer = buffer.getWritePointer(1);

        updateAngles();

        for (size_t sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            amplitudeMultiplier = sin(currentAngle);
            amplitudeMultiplier = ( ( -1 * depth * amplitudeMultiplier ) + ( ( -1 * depth ) + 2 ) ) / 2;
            
            leftBufferPointer[sample] *= amplitudeMultiplier;

            if(isAutoPan)
            {
                amplitudeMultiplier = -1 * amplitudeMultiplier - depth + 2;
            }

            rightBufferPointer[sample] *= amplitudeMultiplier;

            currentAngle += angleDelta;
        }
    }

    void TremoloProcessor::releaseResources()
    {
        currentAngle = 0.f;
        angleDelta = 0.f;
    }
    
    void TremoloProcessor::registerListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void TremoloProcessor::removeListener(juce::AudioProcessorValueTreeState::Listener* listener) const
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void TremoloProcessor::parameterChanged(const juce::String &parameterID, float newValue) 
    {
        updateTremoloParameters();
    }
    
    void TremoloProcessor::updateTremoloParameters()
    {
        depth = apvts.getRawParameterValue("tremoloDepth")->load()/100;
        rate = apvts.getRawParameterValue("tremoloRate")->load();
        isAutoPan = apvts.getRawParameterValue("tremoloAutoPan")->load();
    }

    void TremoloProcessor::updateAngles()
    {
        auto sampleRate = getSampleRate();
        float cyclesPerSample = rate/sampleRate;
        angleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;
    }

    EffectEditor* TremoloProcessor::createEditorUnit()
    {
        return new TremoloEditor(apvts);
    }
}

