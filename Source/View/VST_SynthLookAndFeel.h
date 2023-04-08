/*
==============================================================================

    VST_SynthLookAndFeel.h
    Created: 19 Mar 2023 7:00:29pm
    Author:  Habama10

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

using namespace juce;

class VST_SynthLookAndFeel : public LookAndFeel_V4
{
public:
    VST_SynthLookAndFeel() : LookAndFeel_V4(getMidnightColourScheme())
    {
        setColourScheme( {
                        0xff'2e2e2e, /* windowBackground */
                        0xff'1b1b1b, /* widgetBackground */
                        0xff'2e2e2e, /* menuBackground */
                        0xff'797979, /* outline */
                        0xff'b6b6b6, /* defaultText */
                        0xff'b6b6b6, /* defaultFill */
                        0xff'1a1a1a, /* highlightedText */
                        0xff'cc3401, /* highlightedFill */
                        0xff'b6b6b6  /* menuText */
                        } );

        setColour( juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, juce::Colour( 0x8d'cc3401 ) );
        setColour( juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour( 0xff'cc3401 ) );
    }

    ~VST_SynthLookAndFeel() {}

private:
};
