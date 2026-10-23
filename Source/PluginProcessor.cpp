#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr float kMinCutoff = 30.0f;
constexpr float kMaxCutoff = 18000.0f;

struct CharacterSettings
{
    float filterStart, filterEnd, delayMix, feedback, reverbMix, roomSize, drive;
};

CharacterSettings settingsFor(int character)
{
    switch (character)
    {
        case 1: return { 120.0f, 10500.0f, 0.34f, 0.48f, 0.42f, 0.92f, 3.5f }; // Massive
        case 2: return { 180.0f, 14500.0f, 0.20f, 0.32f, 0.16f, 0.42f, 2.0f }; // Tech
        case 3: return { 70.0f, 8500.0f, 0.40f, 0.38f, 0.55f, 0.98f, 1.4f }; // Atmosphere
        default: return { 90.0f, 12000.0f, 0.28f, 0.40f, 0.28f, 0.72f, 2.5f }; // Classic
    }
}
}

WaveBombzEndlessSmileAudioProcessor::WaveBombzEndlessSmileAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout WaveBombzEndlessSmileAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "INTENSITY", 1 }, "Intensity",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "CHARACTER", 1 }, "Character",
        juce::StringArray { "Classic", "Massive", "Tech", "Atmosphere" }, 0));
    return layout;
}

void WaveBombzEndlessSmileAudioProcessor::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(maximumExpectedSamplesPerBlock),
                                 static_cast<juce::uint32>(getTotalNumOutputChannels()) };
    highPass.prepare(spec);
    delayLeft.prepare(spec);
    delayRight.prepare(spec);
    delayLeft.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0));
    delayRight.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0));
    delayLeft.reset();
    delayRight.reset();
    reverb.reset();
    dryBuffer.setSize(getTotalNumOutputChannels(), maximumExpectedSamplesPerBlock);
    intensitySmoothed.reset(sampleRate, 0.025);
    delaySamplesSmoothed.reset(sampleRate, 0.05);
    intensitySmoothed.setCurrentAndTargetValue(*parameters.getRawParameterValue("INTENSITY"));
    updateFilter(kMinCutoff, 0.707f);
    lastCharacter = -1;
    delayFeedbackStateL = delayFeedbackStateR = 0.0f;
}

void WaveBombzEndlessSmileAudioProcessor::releaseResources()
{
    delayLeft.reset(); delayRight.reset(); reverb.reset();
}

bool WaveBombzEndlessSmileAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void WaveBombzEndlessSmileAudioProcessor::updateFilter(float cutoffHz, float resonance)
{
    const auto cutoff = juce::jlimit(kMinCutoff, static_cast<float>(currentSampleRate * 0.45), cutoffHz);
    auto coeff = Coefficients::makeHighPass(currentSampleRate, cutoff, resonance);
    *highPass.get<0>().state = *coeff;
    *highPass.get<1>().state = *coeff;
}

void WaveBombzEndlessSmileAudioProcessor::updateCharacter(int character, float amount)
{
    const auto s = settingsFor(character);
    const auto cutoff = s.filterStart * std::pow(s.filterEnd / s.filterStart, amount);
    updateFilter(cutoff, 0.707f + 0.6f * amount);
    juce::dsp::Reverb::Parameters rp;
    rp.roomSize = juce::jlimit(0.0f, 1.0f, s.roomSize * amount);
    rp.damping = 0.45f;
    rp.wetLevel = s.reverbMix * amount;
    rp.dryLevel = 0.0f;
    rp.width = 1.0f;
    rp.freezeMode = 0.0f;
    reverb.setParameters(rp);
}

void WaveBombzEndlessSmileAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const auto numSamples = buffer.getNumSamples();
    const auto channels = buffer.getNumChannels();
    if (numSamples == 0 || channels < 2) return;

    const float targetIntensity = parameters.getRawParameterValue("INTENSITY")->load();
    intensitySmoothed.setTargetValue(targetIntensity);
    const int character = static_cast<int>(parameters.getRawParameterValue("CHARACTER")->load());
    const auto settings = settingsFor(character);
    const auto* playHead = getPlayHead();
    double bpm = 120.0;
    if (playHead != nullptr)
        if (const auto position = playHead->getPosition())
            if (const auto tempo = position->getBpm()) bpm = juce::jlimit(40.0, 240.0, *tempo);

    const float targetDelaySamples = static_cast<float>(juce::jlimit(1.0, currentSampleRate * 1.9,
        (60.0 / bpm) * 0.75 * currentSampleRate));
    delaySamplesSmoothed.setTargetValue(targetDelaySamples);
    dryBuffer.makeCopyOf(buffer, true);

    for (int n = 0; n < numSamples; ++n)
    {
        const float amount = intensitySmoothed.getNextValue();
        const float cutoff = settings.filterStart * std::pow(settings.filterEnd / settings.filterStart, amount);
        updateFilter(cutoff, 0.707f + 0.6f * amount);
        const float delaySamples = delaySamplesSmoothed.getNextValue();
        const float delayMix = settings.delayMix * amount;
        const float feedback = settings.feedback * amount;
        const float drive = 1.0f + (settings.drive - 1.0f) * amount;

        float inL = buffer.getSample(0, n);
        float inR = buffer.getSample(1, n);
        float filteredL = highPass.get<0>().processSample(0, inL);
        float filteredR = highPass.get<1>().processSample(1, inR);
        const float tapL = delayLeft.popSample(0, delaySamples, true);
        const float tapR = delayRight.popSample(0, delaySamples, true);
        delayLeft.pushSample(0, filteredL + tapR * feedback);
        delayRight.pushSample(0, filteredR + tapL * feedback);
        float wetL = filteredL * (1.0f - delayMix) + tapL * delayMix;
        float wetR = filteredR * (1.0f - delayMix) + tapR * delayMix;
        wetL = std::tanh(wetL * drive);
        wetR = std::tanh(wetR * drive);
        buffer.setSample(0, n, wetL);
        buffer.setSample(1, n, wetR);
    }

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);

    const float amountNow = targetIntensity;
    const float outputWet = juce::jlimit(0.0f, 1.0f, amountNow);
    for (int ch = 0; ch < channels; ++ch)
        for (int n = 0; n < numSamples; ++n)
            buffer.setSample(ch, n, dryBuffer.getSample(ch, n) * (1.0f - outputWet) + buffer.getSample(ch, n) * outputWet);
}

juce::AudioProcessorEditor* WaveBombzEndlessSmileAudioProcessor::createEditor()
{
    return new WaveBombzEndlessSmileAudioProcessorEditor(*this);
}

void WaveBombzEndlessSmileAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void WaveBombzEndlessSmileAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WaveBombzEndlessSmileAudioProcessor();
}
