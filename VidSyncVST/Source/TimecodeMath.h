#pragma once

#include <cmath>

struct SmpteParts {
    int hours { 0 };
    int minutes { 0 };
    int seconds { 0 };
    int frames { 0 };
};

inline int nominalSmpteRate(double fps) {
    const int rounded = static_cast<int>(std::lround(fps));
    return rounded > 0 ? rounded : 24;
}

// 29.97 and 59.94 are drop-frame. 23.976 is counted with 24 frame numbers
// and does not drop.
inline bool smpteUsesDropFrame(double fps) {
    return std::fabs(fps - 29.97) < 0.01 || std::fabs(fps - 59.94) < 0.02;
}

inline SmpteParts smpteFromFrameCount(long long frameNumber, int nominalRate, bool dropFrame) {
    SmpteParts parts;
    if (nominalRate <= 0)
        nominalRate = 24;
    if (frameNumber < 0)
        frameNumber = 0;

    if (dropFrame && (nominalRate == 30 || nominalRate == 60)) {
        const int dropFrames = nominalRate == 30 ? 2 : 4;
        const int framesPerMinute = nominalRate * 60 - dropFrames;
        const int framesPer10Minutes = nominalRate * 60 * 10 - dropFrames * 9;
        const long long tenMinBlocks = frameNumber / framesPer10Minutes;
        const long long withinTen = frameNumber % framesPer10Minutes;

        if (withinTen > dropFrames) {
            frameNumber += static_cast<long long>(dropFrames) * 9 * tenMinBlocks
                + static_cast<long long>(dropFrames) * ((withinTen - dropFrames) / framesPerMinute);
        } else {
            frameNumber += static_cast<long long>(dropFrames) * 9 * tenMinBlocks;
        }
    }

    parts.frames = static_cast<int>(frameNumber % nominalRate);
    const long long wholeSeconds = frameNumber / nominalRate;
    parts.seconds = static_cast<int>(wholeSeconds % 60);
    const long long wholeMinutes = wholeSeconds / 60;
    parts.minutes = static_cast<int>(wholeMinutes % 60);
    parts.hours = static_cast<int>(wholeMinutes / 60);
    return parts;
}

inline SmpteParts smpteFromSeconds(double seconds, double fps) {
    if (!(seconds > 0.0) || !std::isfinite(seconds))
        seconds = 0.0;
    if (!(fps > 0.0) || !std::isfinite(fps))
        fps = 24.0;

    const long long frameNumber = static_cast<long long>(std::floor(seconds * fps + 1e-4));
    return smpteFromFrameCount(frameNumber, nominalSmpteRate(fps), smpteUsesDropFrame(fps));
}
