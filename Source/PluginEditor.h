#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class WaveBombzEndlessSmileAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit WaveBombzEndlessSmileAudioProcessorEditor(WaveBombzEndlessSmileAudioProcessor&);
    ~WaveBombzEndlessSmileAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    WaveBombzEndlessSmileAudioProcessor& audioProcessor;
    juce::Slider intensity;
    juce::ComboBox character;
    juce::Label title, intensityLabel, characterLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> intensityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> characterAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveBombzEndlessSmileAudioProcessorEditor)
};
