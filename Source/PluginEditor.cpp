#include "PluginEditor.h"

WaveBombzEndlessSmileAudioProcessorEditor::WaveBombzEndlessSmileAudioProcessorEditor(WaveBombzEndlessSmileAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setResizable(true, true);
    setResizeLimits(420, 300, 900, 650);
    setSize(560, 380);

    title.setText("WAVEBOMBZ", juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);
    title.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold)));
    addAndMakeVisible(title);

    intensity.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    intensity.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 24);
    intensity.setTextValueSuffix(" %");
    intensity.setNumDecimalPlacesToDisplay(0);
    intensity.setRange(0.0, 100.0, 0.1);
    intensity.setSkewFactorFromMidPoint(65.0);
    intensity.setLookAndFeel(nullptr);
    addAndMakeVisible(intensity);

    character.addItem("CLASSIC", 1);
    character.addItem("MASSIVE", 2);
    character.addItem("TECH", 3);
    character.addItem("ATMOSPHERE", 4);
    addAndMakeVisible(character);

    intensityLabel.setText("INTENSITY", juce::dontSendNotification);
    intensityLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(intensityLabel);
    characterLabel.setText("CHARACTER", juce::dontSendNotification);
    characterLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(characterLabel);

    intensityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "INTENSITY", intensity);
    characterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.parameters, "CHARACTER", character);
}

WaveBombzEndlessSmileAudioProcessorEditor::~WaveBombzEndlessSmileAudioProcessorEditor() = default;

void WaveBombzEndlessSmileAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(13, 15, 22));
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(25, 29, 40));
    g.fillRoundedRectangle(bounds.reduced(10.0f), 18.0f);
    g.setColour(juce::Colour(0, 220, 190));
    g.drawRoundedRectangle(bounds.reduced(10.0f), 18.0f, 1.5f);
    g.setColour(juce::Colour(0, 220, 190));
    g.fillRoundedRectangle(juce::Rectangle<float>(bounds.getCentreX() - 45.0f, 65.0f, 90.0f, 3.0f), 1.5f);
}

void WaveBombzEndlessSmileAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(28);
    title.setBounds(area.removeFromTop(58));
    area.removeFromTop(12);
    auto knobArea = area.removeFromTop(static_cast<int>(area.getHeight() * 0.68f));
    intensity.setBounds(knobArea.withSizeKeepingCentre(190, 190));
    intensityLabel.setBounds(intensity.getX(), intensity.getBottom() - 4, intensity.getWidth(), 24);
    auto presetArea = area.removeFromTop(72);
    characterLabel.setBounds(presetArea.removeFromTop(24));
    character.setBounds(presetArea.withSizeKeepingCentre(220, 34));
}
