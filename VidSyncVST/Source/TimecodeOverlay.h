#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "TimecodeMath.h"

class TimecodeCalculator {
public:
    static double getFramerate(int choiceIndex) {
        switch (choiceIndex) {
            case 0: return 23.976;
            case 1: return 24.0;
            case 2: return 25.0;
            case 3: return 29.97;
            case 4: return 30.0;
            case 5: return 60.0;
            default: return 24.0;
        }
    }

    static juce::String formatSMPTE(double seconds, double fps) {
        const SmpteParts parts = smpteFromSeconds(seconds, fps);
        return juce::String::formatted("%02d:%02d:%02d:%02d",
                                       parts.hours,
                                       parts.minutes,
                                       parts.seconds,
                                       parts.frames);
    }
};
