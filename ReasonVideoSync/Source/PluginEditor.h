#pragma once
#include "PluginProcessor.h"
#include "MacVideoView.h"

class ReasonVideoSyncAudioProcessorEditor : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    explicit ReasonVideoSyncAudioProcessorEditor(ReasonVideoSyncAudioProcessor&);
    ~ReasonVideoSyncAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void openFileBrowser();

    ReasonVideoSyncAudioProcessor& audioProcessor;
    MacVideoView videoView;
    juce::TextButton openButton { "Load Video..." };
    juce::Label timecodeLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;

    double lastPolledTime = -1.0;
    bool lastPlayState = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReasonVideoSyncAudioProcessorEditor)
};
