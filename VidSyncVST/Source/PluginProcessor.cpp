#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout VidSyncAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"offset", 1},
        "Start Offset (sec)",
        juce::NormalisableRange<float>(-3600.0f, 3600.0f, 0.01f),
        0.0f
    ));

    juce::StringArray fpsOptions { "23.976 fps", "24 fps (Film)", "25 fps (PAL)", "29.97 fps (NTSC)", "30 fps", "60 fps" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"fps", 1},
        "SMPTE Frame Rate",
        fpsOptions,
        1
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"volume", 1},
        "Video Audio Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"mute", 1},
        "Mute Video Audio",
        false
    ));

    return { params.begin(), params.end() };
}

VidSyncAudioProcessor::VidSyncAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

VidSyncAudioProcessor::~VidSyncAudioProcessor() = default;

const juce::String VidSyncAudioProcessor::getName() const { return "VidSyncVST"; }
bool VidSyncAudioProcessor::acceptsMidi() const { return false; }
bool VidSyncAudioProcessor::producesMidi() const { return false; }
bool VidSyncAudioProcessor::isMidiEffect() const { return false; }
double VidSyncAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int VidSyncAudioProcessor::getNumPrograms() { return 1; }
int VidSyncAudioProcessor::getCurrentProgram() { return 0; }
void VidSyncAudioProcessor::setCurrentProgram(int) {}
const juce::String VidSyncAudioProcessor::getProgramName(int) { return {}; }
void VidSyncAudioProcessor::changeProgramName(int, const juce::String&) {}

void VidSyncAudioProcessor::prepareToPlay(double, int) {}
void VidSyncAudioProcessor::releaseResources() {}

bool VidSyncAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto& input = layouts.getMainInputChannelSet();
    const auto& output = layouts.getMainOutputChannelSet();
    // Reason probes channel layouts while opening the device. The default
    // layout stays stereo; mono is accepted when the host asks for it.
    if (input != output)
        return false;
    return input == juce::AudioChannelSet::mono()
        || input == juce::AudioChannelSet::stereo();
}

void VidSyncAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    // This is an insert effect. Preserve the incoming audio instead of clearing it.
    // Clear only any output channels that do not have a matching input channel.
    const auto inputChannels = getTotalNumInputChannels();
    const auto outputChannels = getTotalNumOutputChannels();
    for (int channel = inputChannels; channel < outputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    if (auto* playHead = getPlayHead()) {
        if (auto posInfo = playHead->getPosition()) {
            syncValid.store(true);
            hostIsPlaying.store(posInfo->getIsPlaying());

            if (auto timeSec = posInfo->getTimeInSeconds())
                hostTimeSeconds.store(*timeSec);

            if (auto ppq = posInfo->getPpqPosition())
                hostPpq.store(*ppq);

            if (auto bpm = posInfo->getBpm())
                hostBpm.store(*bpm);
        } else {
            syncValid.store(false);
        }
    } else {
        syncValid.store(false);
    }
}

PlayheadSyncState VidSyncAudioProcessor::getSyncState() const {
    PlayheadSyncState state;
    state.valid = syncValid.load();
    state.isPlaying = hostIsPlaying.load();
    state.timeInSeconds = hostTimeSeconds.load();
    state.ppqPosition = hostPpq.load();
    state.bpm = hostBpm.load();
    return state;
}

bool VidSyncAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* VidSyncAudioProcessor::createEditor() {
    return new VidSyncAudioProcessorEditor(*this);
}

void VidSyncAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    state.setProperty("videoPath", lastLoadedFilePath, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void VidSyncAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType())) {
        auto state = juce::ValueTree::fromXml(*xmlState);
        apvts.replaceState(state);
        lastLoadedFilePath = state.getProperty("videoPath", "").toString();
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new VidSyncAudioProcessor();
}
