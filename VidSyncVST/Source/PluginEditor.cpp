#include "PluginEditor.h"

VidSyncAudioProcessorEditor::VidSyncAudioProcessorEditor(VidSyncAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Configure Video Component
    addAndMakeVisible(videoComponent);

    videoComponent.onFileLoaded = [this](const juce::File& file) {
        processorRef.setLastLoadedFilePath(file.getFullPathName());
    };

    // Restore last loaded file if present
    auto lastFile = processorRef.getLastLoadedFilePath();
    if (lastFile.isNotEmpty()) {
        juce::File f(lastFile);
        if (f.existsAsFile())
            videoComponent.loadVideoFile(f);
    }

    // Offset Slider
    offsetSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    offsetSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 20);
    offsetSlider.setTextValueSuffix(" s");
    addAndMakeVisible(offsetSlider);
    offsetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "offset", offsetSlider);

    offsetLabel.setText("Cue Offset:", juce::dontSendNotification);
    offsetLabel.setFont(juce::Font(13.0f));
    offsetLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(offsetLabel);

    // Frame rate ComboBox
    fpsComboBox.addItemList({ "23.976 fps", "24 fps", "25 fps", "29.97 fps", "30 fps", "60 fps" }, 1);
    addAndMakeVisible(fpsComboBox);
    fpsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processorRef.apvts, "fps", fpsComboBox);

    fpsLabel.setText("Framerate:", juce::dontSendNotification);
    fpsLabel.setFont(juce::Font(13.0f));
    fpsLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(fpsLabel);

    // Volume Slider
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    addAndMakeVisible(volumeSlider);
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "volume", volumeSlider);

    volumeLabel.setText("Audio Vol:", juce::dontSendNotification);
    volumeLabel.setFont(juce::Font(13.0f));
    volumeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(volumeLabel);

    // Mute Button
    muteToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(muteToggle);
    muteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.apvts, "mute", muteToggle);

    // Editor window setup
    setResizable(true, true);
    setResizeLimits(640, 420, 1920, 1200);
    setSize(800, 520);

    // Start 60Hz UI sync timer
    startTimerHz(60);
}

VidSyncAudioProcessorEditor::~VidSyncAudioProcessorEditor() {
    stopTimer();
}

void VidSyncAudioProcessorEditor::timerCallback() {
    auto sync = processorRef.getSyncState();
    auto offsetParam = processorRef.apvts.getRawParameterValue("offset");
    auto fpsParam = processorRef.apvts.getRawParameterValue("fps");
    auto volParam = processorRef.apvts.getRawParameterValue("volume");
    auto muteParam = processorRef.apvts.getRawParameterValue("mute");

    double offset = offsetParam ? offsetParam->load() : 0.0;
    int fpsIdx = fpsParam ? static_cast<int>(fpsParam->load()) : 1;
    float vol = volParam ? volParam->load() : 1.0f;
    bool isMuted = muteParam ? (muteParam->load() > 0.5f) : false;

    // Push playback sync to video engine
    videoComponent.updatePlaybackSync(sync.timeInSeconds, sync.isPlaying, offset, fpsIdx);
    videoComponent.updateAudioSettings(vol, isMuted);
}

void VidSyncAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff18181c));

    // Bottom parameter strip background
    g.setColour(juce::Colour(0xff141416));
    g.fillRect(0, getHeight() - 48, getWidth(), 48);

    g.setColour(juce::Colour(0xff2a2a32));
    g.drawHorizontalLine(getHeight() - 48, 0.0f, static_cast<float>(getWidth()));
}

void VidSyncAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();
    auto bottomBar = bounds.removeFromBottom(48).reduced(8, 4);

    // Position Video Component
    videoComponent.setBounds(bounds);

    // Position Bottom Controls
    offsetLabel.setBounds(bottomBar.removeFromLeft(75));
    offsetSlider.setBounds(bottomBar.removeFromLeft(160));
    bottomBar.removeFromLeft(12);

    fpsLabel.setBounds(bottomBar.removeFromLeft(70));
    fpsComboBox.setBounds(bottomBar.removeFromLeft(100).reduced(0, 5));
    bottomBar.removeFromLeft(16);

    volumeLabel.setBounds(bottomBar.removeFromLeft(68));
    volumeSlider.setBounds(bottomBar.removeFromLeft(120));
    bottomBar.removeFromLeft(12);

    muteToggle.setBounds(bottomBar.removeFromLeft(65).reduced(0, 5));
}
