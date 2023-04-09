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
                        0xff'2e2044, /* windowBackground */
                        0x73'000000, /* widgetBackground */
                        0xff'2e2044, /* menuBackground */
                        0x5e'007956, /* outline */
                        0xff'ffffff, /* defaultText */
                        0xff'c3dad2, /* defaultFill */
                        0xff'ffffff, /* highlightedText */
                        0xff'36a785, /* highlightedFill */
                        0xff'ffffff  /* menuText */
                        } );

        setColour( juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, juce::Colour( 0x8d'00a876 ) );
        setColour( juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour( 0xff'00a876 ) );
    }

    ~VST_SynthLookAndFeel() {}

private:
};
