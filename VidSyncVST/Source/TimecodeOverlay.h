#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

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
        if (seconds < 0.0) seconds = 0.0;
        int totalFrames = static_cast<int>(seconds * fps + 0.0001);

        int f = totalFrames % static_cast<int>(fps);
        int totalSeconds = totalFrames / static_cast<int>(fps);

        int s = totalSeconds % 60;
        int totalMinutes = totalSeconds / 60;

        int m = totalMinutes % 60;
        int h = totalMinutes / 60;

        return juce::String::formatted("%02d:%02d:%02d:%02d", h, m, s, f);
    }
};
