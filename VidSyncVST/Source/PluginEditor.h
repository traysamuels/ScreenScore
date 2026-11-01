#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "VideoPlayerComponent.h"

class VidSyncAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    public juce::Timer
{
public:
    explicit VidSyncAudioProcessorEditor(VidSyncAudioProcessor&);
    ~VidSyncAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    VidSyncAudioProcessor& processorRef;

    VideoPlayerComponent videoComponent;

    // Controls toolbar
    juce::Slider offsetSlider;
    juce::Label offsetLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> offsetAttachment;

    juce::ComboBox fpsComboBox;
    juce::Label fpsLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> fpsAttachment;

    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;

    juce::ToggleButton muteToggle { "Mute" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VidSyncAudioProcessorEditor)
};
