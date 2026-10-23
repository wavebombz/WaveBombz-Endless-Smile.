#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

class WaveBombzEndlessSmileAudioProcessor final : public juce::AudioProcessor
{
public:
    WaveBombzEndlessSmileAudioProcessor();
    ~WaveBombzEndlessSmileAudioProcessor() override = default;
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    Filter highPass1L, highPass1R, highPass2L, highPass2R;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLeft { 192000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayRight { 192000 };
    juce::dsp::Reverb reverb;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> intensitySmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delaySamplesSmoothed;
    juce::AudioBuffer<float> dryBuffer;
    double currentSampleRate { 44100.0 };
    void updateFilter(float cutoffHz, float resonance);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveBombzEndlessSmileAudioProcessor)
};
