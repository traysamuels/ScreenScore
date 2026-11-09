#include "TimecodeMath.h"

#include <cstdio>

namespace {
int gFailures = 0;

void expectParts(const char* label, SmpteParts actual, int h, int m, int s, int f) {
    if (actual.hours == h && actual.minutes == m && actual.seconds == s && actual.frames == f)
        return;

    std::fprintf(stderr, "FAIL %s: got %02d:%02d:%02d:%02d, expected %02d:%02d:%02d:%02d\n",
                 label,
                 actual.hours, actual.minutes, actual.seconds, actual.frames,
                 h, m, s, f);
    ++gFailures;
}
}

int main() {
    expectParts("zero", smpteFromFrameCount(0, 30, true), 0, 0, 0, 0);
    expectParts("df last frame of first minute", smpteFromFrameCount(1799, 30, true), 0, 0, 59, 29);
    expectParts("df first displayed frame of minute 1", smpteFromFrameCount(1800, 30, true), 0, 1, 0, 2);
    expectParts("df ten minute boundary", smpteFromFrameCount(17982, 30, true), 0, 10, 0, 0);
    expectParts("non-drop one second", smpteFromFrameCount(24, 24, false), 0, 0, 1, 0);
    expectParts("non-drop frame 23", smpteFromFrameCount(23, 24, false), 0, 0, 0, 23);

    // 23.976 must count 0-23, not 0-22. The old cast-to-int rate made
    // frame 23 roll over a second early.
    expectParts("23.976 one frame before the next second",
                smpteFromSeconds(23.0 / 23.976, 23.976), 0, 0, 0, 23);
    expectParts("23.976 exact nominal second",
                smpteFromSeconds(24.0 / 23.976, 23.976), 0, 0, 1, 0);
    expectParts("24 fps one second", smpteFromSeconds(1.0, 24.0), 0, 0, 1, 0);
    expectParts("25 fps one second", smpteFromSeconds(1.0, 25.0), 0, 0, 1, 0);
    expectParts("30 fps 1.5s", smpteFromSeconds(1.5, 30.0), 0, 0, 1, 15);
    expectParts("negative clamps", smpteFromSeconds(-4.0, 24.0), 0, 0, 0, 0);

    if (gFailures != 0) {
        std::fprintf(stderr, "%d timecode checks failed\n", gFailures);
        return 1;
    }

    std::puts("timecode checks passed");
    return 0;
}
