/*
==============================================================================

    TripleBuffer.h
    Created: 2 May 2023 5:16:13pm
    Author:  Habama10

==============================================================================
*/

#pragma once

namespace Utils
{
    // The source of this code: https://forum.juce.com/t/parameter-changes-thread-safety/50098/14
    template <typename T>
    struct TripleBuffer
    {
        auto& read() const { return buf_[r_]; }
        auto& write()      { return buf_[w_]; }

        bool acquire()
        {
            int changed{ ready_.load (std::memory_order_relaxed) & 4 };
            if (changed) r_ = ready_.exchange (r_, std::memory_order_acquire) & 3;
            return changed;
        }

        void release() { w_ = ready_.exchange (w_ | 4, std::memory_order_release) & 3; };

    private:
        T buf_[3]{};
        int r_{ 1 }, w_{ 2 };
        std::atomic_int ready_{};
    };
}