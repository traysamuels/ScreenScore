#include "PluginProcessor.h"
#include "PluginEditor.h"

ReasonVideoSyncAudioProcessor::ReasonVideoSyncAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

ReasonVideoSyncAudioProcessor::~ReasonVideoSyncAudioProcessor() {}

void ReasonVideoSyncAudioProcessor::prepareToPlay(double, int) {}
void ReasonVideoSyncAudioProcessor::releaseResources() {}

void ReasonVideoSyncAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(buffer);

    if (auto* playHead = getPlayHead())
    {
        if (auto pos = playHead->getPosition())
        {
            if (auto timeInSec = pos->getTimeInSeconds())
                currentTimelineSeconds.store(*timeInSec, std::memory_order_relaxed);

            if (auto playing = pos->getIsPlaying())
                isHostPlaying.store(*playing, std::memory_order_relaxed);
        }
    }
}

juce::AudioProcessorEditor* ReasonVideoSyncAudioProcessor::createEditor()
{
    return new ReasonVideoSyncAudioProcessorEditor(*this);
}

void ReasonVideoSyncAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    stream.writeString(currentVideoFile.getFullPathName());
}

void ReasonVideoSyncAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    currentVideoFile = juce::File(stream.readString());
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReasonVideoSyncAudioProcessor();
}
