#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr float kMinCutoff = 30.0f;
struct CharacterSettings { float filterStart, filterEnd, delayMix, feedback, reverbMix, roomSize, drive; };
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
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout()) {}

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
    juce::dsp::ProcessSpec monoSpec { sampleRate, static_cast<juce::uint32>(maximumExpectedSamplesPerBlock), 1 };
    highPass1L.prepare(monoSpec); highPass1R.prepare(monoSpec);
    highPass2L.prepare(monoSpec); highPass2R.prepare(monoSpec);
    delayLeft.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0));
    delayRight.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0));
    juce::dsp::ProcessSpec stereoSpec { sampleRate, static_cast<juce::uint32>(maximumExpectedSamplesPerBlock), 2 };
    delayLeft.prepare(stereoSpec); delayRight.prepare(stereoSpec);
    delayLeft.reset(); delayRight.reset(); reverb.reset();
    dryBuffer.setSize(2, maximumExpectedSamplesPerBlock);
    intensitySmoothed.reset(sampleRate, 0.025);
    delaySamplesSmoothed.reset(sampleRate, 0.05);
    intensitySmoothed.setCurrentAndTargetValue(*parameters.getRawParameterValue("INTENSITY"));
    updateFilter(30.0f, 0.707f);
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
    const auto coeff = Coefficients::makeHighPass(currentSampleRate, cutoff, resonance);
    highPass1L.coefficients = coeff; highPass1R.coefficients = coeff;
    highPass2L.coefficients = coeff; highPass2R.coefficients = coeff;
}
void WaveBombzEndlessSmileAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0 || buffer.getNumChannels() < 2) return;
    const float targetIntensity = parameters.getRawParameterValue("INTENSITY")->load();
    intensitySmoothed.setTargetValue(targetIntensity);
    const int character = static_cast<int>(parameters.getRawParameterValue("CHARACTER")->load());
    const auto settings = settingsFor(character);
    double bpm = 120.0;
    if (auto* playHead = getPlayHead())
        if (const auto position = playHead->getPosition())
            if (const auto tempo = position->getBpm()) bpm = juce::jlimit(40.0, 240.0, *tempo);
    const float targetDelaySamples = static_cast<float>(juce::jlimit(1.0, currentSampleRate * 1.9,
        (60.0 / bpm) * 0.75 * currentSampleRate));
    delaySamplesSmoothed.setTargetValue(targetDelaySamples);
    dryBuffer.makeCopyOf(buffer, true);

    // Update filter coefficients once per block to avoid allocations in the audio loop.
    const float blockIntensity = intensitySmoothed.getCurrentValue();
    const float cutoff = settings.filterStart * std::pow(settings.filterEnd / settings.filterStart, blockIntensity);
    updateFilter(cutoff, 0.707f + 0.6f * blockIntensity);
    juce::dsp::Reverb::Parameters rp;
    rp.roomSize = juce::jlimit(0.0f, 1.0f, settings.roomSize * blockIntensity);
    rp.damping = 0.45f; rp.wetLevel = settings.reverbMix * blockIntensity;
    rp.dryLevel = 0.0f; rp.width = 1.0f; rp.freezeMode = 0.0f;
    reverb.setParameters(rp);

    for (int n = 0; n < numSamples; ++n)
    {
        const float amount = intensitySmoothed.getNextValue();
        const float delaySamples = delaySamplesSmoothed.getNextValue();
        const float delayMix = settings.delayMix * amount;
        const float feedback = settings.feedback * amount;
        const float drive = 1.0f + (settings.drive - 1.0f) * amount;
        const float inL = buffer.getSample(0, n), inR = buffer.getSample(1, n);
        const float filteredL = highPass2L.processSample(highPass1L.processSample(inL));
        const float filteredR = highPass2R.processSample(highPass1R.processSample(inR));
        const float tapL = delayLeft.popSample(0, delaySamples, true);
        const float tapR = delayRight.popSample(0, delaySamples, true);
        delayLeft.pushSample(0, filteredL + tapR * feedback);
        delayRight.pushSample(0, filteredR + tapL * feedback);
        buffer.setSample(0, n, std::tanh((filteredL * (1.0f - delayMix) + tapL * delayMix) * drive));
        buffer.setSample(1, n, std::tanh((filteredR * (1.0f - delayMix) + tapR * delayMix) * drive));
    }
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
    const float outputWet = juce::jlimit(0.0f, 1.0f, targetIntensity);
    for (int ch = 0; ch < 2; ++ch)
        for (int n = 0; n < numSamples; ++n)
            buffer.setSample(ch, n, dryBuffer.getSample(ch, n) * (1.0f - outputWet)
                                      + buffer.getSample(ch, n) * outputWet);
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
