#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

class MacVideoView : public juce::NSViewComponent
{
public:
    MacVideoView();
    ~MacVideoView() override;

    void loadVideoFile(const juce::File& file);
    void setPosition(double seconds, bool isPlaying);
    double getDuration() const;

private:
    void* nativePlayerController = nullptr;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacVideoView)
};
