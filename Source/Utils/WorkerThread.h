/*
==============================================================================

    WorkerThread.h
    Created: 22 Apr 2023 3:12:03am
    Author:  Habama10

==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct WorkerThread : public juce::Thread
{
    WorkerThread(std::function<void()> func) :
        juce::Thread("Worker"),
        func(func)
    {}

    void run() override
    {
        func();
    }
private:
    std::function<void()> func;
};