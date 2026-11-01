#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "MacVideoView.h"
#include "TimecodeOverlay.h"
#include "DetachedVideoWindow.h"

class VideoPlayerComponent : public juce::Component {
public:
    VideoPlayerComponent();
    ~VideoPlayerComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    bool loadVideoFile(const juce::File& file);
    void updatePlaybackSync(double dawSeconds, bool isPlaying, double offsetSeconds, int fpsChoiceIndex);
    void updateAudioSettings(float volume, bool isMuted);

    void togglePopoutWindow();
    bool isPoppedOut() const { return detachedWindow != nullptr; }

    std::function<void(const juce::File&)> onFileLoaded;

private:
    MacVideoBackend videoBackend;
    std::unique_ptr<juce::NSViewComponent> nsViewContainer;
    std::unique_ptr<DetachedVideoWindow> detachedWindow;

    // Controls
    juce::TextButton openButton { "Open Video File..." };
    juce::TextButton popoutButton { "Pop Out Window" };
    juce::Label statusLabel;
    juce::Label timecodeLabel;

    juce::String currentSmpteTimecode { "00:00:00:00" };
    bool currentIsPlaying { false };
    bool hasVideoLoaded { false };

    void setupNativeView();
    void returnViewFromPopout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VideoPlayerComponent)
};
