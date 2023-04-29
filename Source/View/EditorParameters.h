/*
==============================================================================

    EditorParameters.h
    Created: 31 Mar 2023 12:00:18am
    Author:  Habama10

==============================================================================
*/

#pragma once

namespace Editor
{
    //============== MAIN ==============//
    constexpr int WIDTH_MAIN_WINDOW_PX = 750;
    constexpr int HEIGHT_MAIN_WINDOW_PX = 650;

    constexpr int PADDING_PX = 5;

    constexpr float OUTLINE_WIDTH = 2.f;

    constexpr int LABEL_HEIGHT = 20;

    constexpr int IDX_OFFSET = 1;   //necessary for combo boxes

    //============== OSC ==============//
    constexpr int HEIGHT_PARTIAL_PHASE_PX = 45;
    constexpr int WIDTH_PARTIAL_SLIDERS_PX = 60;

    //============== SYNTH ==============//
    constexpr int HEIGHT_TUNING_KNOB_PX = 110;
    constexpr int HEIGHT_UNISON_KNOB_PX = 115;

    constexpr int WIDTH_WAVEFORM_SELECTOR_PX = 120;
    constexpr int HEIGHT_WAVEFORM_SELECTOR_PX = 50;

    constexpr float LEVEL_METER_UPPER_LIMIT = 0.f;
    constexpr float LEVEL_METER_LOWER_LIMIT = -60.f;
}