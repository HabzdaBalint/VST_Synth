/*
==============================================================================

    TremoloUnit.cpp
    Created: 11 Apr 2023 9:39:14pm
    Author:  Habama10

==============================================================================
*/

#include "TremoloUnit.h"

#include "../../../View/Effects/EffectEditors/Tremolo/TremoloEditor.h"

namespace Effects::Tremolo
{
    TremoloUnit::TremoloUnit(juce::AudioProcessorValueTreeState& apvts) : EffectProcessorUnit(apvts)
    {
        registerListener(this);
    }

    TremoloUnit::~TremoloUnit() 
    {
        removeListener(this);
    }

    void TremoloUnit::prepareToPlay(double sampleRate, int samplesPerBlock) 
    {
        setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(), sampleRate, samplesPerBlock);

        juce::dsp::ProcessSpec processSpec;
        processSpec.maximumBlockSize = samplesPerBlock;
        processSpec.numChannels = getMainBusNumOutputChannels();
        processSpec.sampleRate = sampleRate;
        updateTremoloParameters();
    }

    void TremoloUnit::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) 
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

    void TremoloUnit::releaseResources() 
    {
        dryWetMixer.reset();
    }
    
    void TremoloUnit::registerListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.addParameterListener(id, listener);
        }
    }

    void TremoloUnit::removeListener(juce::AudioProcessorValueTreeState::Listener* listener)
    {
        auto paramLayoutSchema = createParameterLayout();
        auto params = paramLayoutSchema->getParameters(false);

        for(auto param : params)
        {
            auto id = dynamic_cast<juce::RangedAudioParameter*>(param)->getParameterID();
            apvts.removeParameterListener(id, listener);
        }
    }

    void TremoloUnit::parameterChanged(const juce::String &parameterID, float newValue) 
    {
        updateTremoloParameters();
    }
    
    void TremoloUnit::updateTremoloParameters()
    {
        depth = apvts.getRawParameterValue("tremoloDepth")->load()/100;
        rate = apvts.getRawParameterValue("tremoloRate")->load();
        isAutoPan = apvts.getRawParameterValue("tremoloAutoPan")->load();
    }

    void TremoloUnit::updateAngles()
    {
        auto sampleRate = getSampleRate();
        float cyclesPerSample = rate/sampleRate;
        angleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;
    }

    EffectEditorUnit* TremoloUnit::createEditorUnit()
    {
        return new TremoloEditor(apvts);
    }
}

