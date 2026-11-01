#include "VideoPlayerComponent.h"

VideoPlayerComponent::VideoPlayerComponent() {
    setupNativeView();

    openButton.onClick = [this]() {
        auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
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
