/*
==============================================================================

    WorkerThread.h
    Created: 22 Apr 2023 3:12:03am
    Author:  Habama10

==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace Utils
{
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WorkerThread)
    };
}