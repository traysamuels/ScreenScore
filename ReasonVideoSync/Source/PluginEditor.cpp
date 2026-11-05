#include "PluginEditor.h"
#include <cmath>

ReasonVideoSyncAudioProcessorEditor::ReasonVideoSyncAudioProcessorEditor(ReasonVideoSyncAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(720, 480);
    setResizable(true, true);
    setResizeLimits(320, 240, 1920, 1080);

    addAndMakeVisible(videoView);
    openButton.onClick = [this] { openFileBrowser(); };
    addAndMakeVisible(openButton);

    timecodeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    timecodeLabel.setText("00:00:00.00", juce::dontSendNotification);
    addAndMakeVisible(timecodeLabel);

    if (audioProcessor.currentVideoFile.existsAsFile())
        videoView.loadVideoFile(audioProcessor.currentVideoFile);

    startTimerHz(30);
}

ReasonVideoSyncAudioProcessorEditor::~ReasonVideoSyncAudioProcessorEditor()
{
    stopTimer();
}

void ReasonVideoSyncAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff181818));
}

void ReasonVideoSyncAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    auto topBar = area.removeFromTop(40);
    openButton.setBounds(topBar.removeFromLeft(120).reduced(5));
    timecodeLabel.setBounds(topBar.removeFromLeft(140).reduced(5));
    videoView.setBounds(area);
}

void ReasonVideoSyncAudioProcessorEditor::timerCallback()
{
    const double curTime = audioProcessor.currentTimelineSeconds.load(std::memory_order_relaxed);
    const bool isPlaying = audioProcessor.isHostPlaying.load(std::memory_order_relaxed);

    if (std::abs(curTime - lastPolledTime) > 0.03 || isPlaying != lastPlayState)
    {
        videoView.setPosition(curTime, isPlaying);
        lastPolledTime = curTime;
        lastPlayState = isPlaying;

        const int mins = static_cast<int>(curTime / 60.0);
        const int secs = static_cast<int>(curTime) % 60;
        const int centiseconds = static_cast<int>((curTime - std::floor(curTime)) * 100.0);
        timecodeLabel.setText(juce::String::formatted("%02d:%02d.%02d", mins, secs, centiseconds),
                              juce::dontSendNotification);
    }
}

void ReasonVideoSyncAudioProcessorEditor::openFileBrowser()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select a Video File to Sync with Reason...",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.mp4;*.mov;*.m4v;*.mkv");

    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc)
        {
            const auto file = fc.getResult();
            if (file.existsAsFile())
            {
                audioProcessor.currentVideoFile = file;
                videoView.loadVideoFile(file);
            }
        });
}
