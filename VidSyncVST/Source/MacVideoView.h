#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

class MacVideoBackend {
public:
    MacVideoBackend();
    ~MacVideoBackend();

    bool loadFile(const juce::File& videoFile);
    void sync(double dawSeconds, bool isPlaying, double offsetSeconds);
    void seek(double targetSeconds);
    void setVolume(float volume, bool muted);

    double getDuration() const;
    juce::Point<int> getNaturalSize() const;
    void* getNativeView() const; // Returns NSView*
    bool isLoaded() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
