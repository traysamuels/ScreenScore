#include "VideoPlayerComponent.h"

VideoPlayerComponent::VideoPlayerComponent() {
    setupNativeView();

    openButton.onClick = [this]() {
        auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canFiles;
        auto chooser = std::make_shared<juce::FileChooser>("Select Video File (MP4, MOV, ProRes, MKV)...",
                                                            juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                                                            "*.mp4;*.mov;*.m4v;*.mkv;*.avi");
        chooser->launchAsync(chooserFlags, [this, chooser](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file.existsAsFile()) {
                if (loadVideoFile(file)) {
                    if (onFileLoaded) onFileLoaded(file);
                }
            }
        });
    };
    addAndMakeVisible(openButton);

    popoutButton.onClick = [this]() { togglePopoutWindow(); };
    addAndMakeVisible(popoutButton);

    statusLabel.setFont(juce::Font(13.0f, juce::Font::bold));
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    statusLabel.setText("No video loaded", juce::dontSendNotification);
    addAndMakeVisible(statusLabel);

    timecodeLabel.setFont(juce::Font("Monaco", 18.0f, juce::Font::bold));
    timecodeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00ffcc));
    timecodeLabel.setText("00:00:00:00", juce::dontSendNotification);
    timecodeLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(timecodeLabel);
}

VideoPlayerComponent::~VideoPlayerComponent() {
    if (detachedWindow != nullptr) {
        detachedWindow->clearContentComponent();
        detachedWindow = nullptr;
    }
}

void VideoPlayerComponent::setupNativeView() {
    if (auto* nativeView = videoBackend.getNativeView()) {
        nsViewContainer = std::make_unique<juce::NSViewComponent>();
        nsViewContainer->setView(nativeView);
        addAndMakeVisible(*nsViewContainer);
    }
}

bool VideoPlayerComponent::loadVideoFile(const juce::File& file) {
    if (videoBackend.loadFile(file)) {
        hasVideoLoaded = true;
        auto dims = videoBackend.getNaturalSize();
        statusLabel.setText(file.getFileName() + " (" + juce::String(dims.x) + "x" + juce::String(dims.y) + ")", juce::dontSendNotification);
        resized();
        return true;
    }
    statusLabel.setText("Failed to load video", juce::dontSendNotification);
    return false;
}

void VideoPlayerComponent::updatePlaybackSync(double dawSeconds, bool isPlaying, double offsetSeconds, int fpsChoiceIndex) {
    currentIsPlaying = isPlaying;
    videoBackend.sync(dawSeconds, isPlaying, offsetSeconds);

    double displayTime = dawSeconds - offsetSeconds;
    if (displayTime < 0.0) displayTime = 0.0;
    double fps = TimecodeCalculator::getFramerate(fpsChoiceIndex);
    currentSmpteTimecode = TimecodeCalculator::formatSMPTE(displayTime, fps);
    timecodeLabel.setText(currentSmpteTimecode, juce::dontSendNotification);

    repaint();
}

void VideoPlayerComponent::updateAudioSettings(float volume, bool isMuted) {
    videoBackend.setVolume(volume, isMuted);
}

void VideoPlayerComponent::togglePopoutWindow() {
    if (detachedWindow == nullptr) {
        // Detach to floating window
        detachedWindow = std::make_unique<DetachedVideoWindow>("VidSync Video Monitor", [this]() {
            returnViewFromPopout();
        });

        if (nsViewContainer != nullptr) {
            removeChildComponent(nsViewContainer.get());
            detachedWindow->setContentNonOwned(nsViewContainer.get(), true);
            detachedWindow->setBounds(100, 100, 960, 540);
            detachedWindow->setVisible(true);
        }
        popoutButton.setButtonText("Dock Video");
    } else {
        returnViewFromPopout();
    }
    resized();
}

void VideoPlayerComponent::returnViewFromPopout() {
    if (detachedWindow != nullptr) {
        detachedWindow->clearContentComponent();
        detachedWindow = nullptr;
        if (nsViewContainer != nullptr) {
            addAndMakeVisible(*nsViewContainer);
        }
        popoutButton.setButtonText("Pop Out Window");
        resized();
    }
}

void VideoPlayerComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff18181c));

    // Draw header background
    g.setColour(juce::Colour(0xff22222a));
    g.fillRect(0, 0, getWidth(), 46);

    // Transport Status Light
    auto statusBounds = juce::Rectangle<float>(14.0f, 16.0f, 14.0f, 14.0f);
    if (currentIsPlaying) {
        g.setColour(juce::Colour(0xff00e676)); // Vivid Green
        g.fillEllipse(statusBounds);
        g.setColour(juce::Colour(0x6600e676));
        g.drawEllipse(statusBounds.expanded(2.0f), 2.0f);
    } else {
        g.setColour(juce::Colour(0xffff5252)); // Red Stop
        g.fillEllipse(statusBounds);
    }

    if (!hasVideoLoaded && detachedWindow == nullptr) {
        g.setColour(juce::Colour(0xff2a2a35));
        auto videoArea = getLocalBounds().withTrimmedTop(46);
        g.fillRect(videoArea);

        g.setColour(juce::Colours::grey);
        g.setFont(juce::Font(16.0f, juce::Font::bold));
        g.drawText("No Video Loaded
Click 'Open Video File' above to load a movie reel",
                   videoArea, juce::Justification::centred, true);
    }
}

void VideoPlayerComponent::resized() {
    auto bounds = getLocalBounds();
    auto header = bounds.removeFromTop(46);

    header.removeFromLeft(36); // Space after transport light
    openButton.setBounds(header.removeFromLeft(140).reduced(0, 7));
    header.removeFromLeft(8);
    popoutButton.setBounds(header.removeFromLeft(120).reduced(0, 7));
    header.removeFromLeft(12);

    timecodeLabel.setBounds(header.removeFromRight(150).reduced(0, 5));
    statusLabel.setBounds(header.reduced(0, 5));

    if (detachedWindow == nullptr && nsViewContainer != nullptr) {
        nsViewContainer->setBounds(bounds);
    }
}
