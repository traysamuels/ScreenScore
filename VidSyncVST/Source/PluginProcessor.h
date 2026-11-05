#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

struct PlayheadSyncState {
    bool isPlaying { false };
    double timeInSeconds { 0.0 };
    double ppqPosition { 0.0 };
    double bpm { 120.0 };
    bool isLooping { false };
    double loopStart { 0.0 };
    double loopEnd { 0.0 };
    bool valid { false };
};

class VidSyncAudioProcessor : public juce::AudioProcessor {
public:
    VidSyncAudioProcessor();
    ~VidSyncAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    PlayheadSyncState getSyncState() const;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::String getLastLoadedFilePath() const { return lastLoadedFilePath; }
    void setLastLoadedFilePath(const juce::String& path) { lastLoadedFilePath = path; }

private:
    std::atomic<bool> hostIsPlaying { false };
    std::atomic<double> hostTimeSeconds { 0.0 };
    std::atomic<double> hostPpq { 0.0 };
    std::atomic<double> hostBpm { 120.0 };
    std::atomic<bool> syncValid { false };

    juce::String lastLoadedFilePath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VidSyncAudioProcessor)
};
