#include "PluginEditor.h"

namespace
{
    const juce::Colour background(9, 12, 19);
    const juce::Colour panel(20, 25, 35);
    const juce::Colour panelLight(28, 35, 47);
    const juce::Colour teal(0, 226, 194);
    const juce::Colour muted(142, 157, 174);
    const juce::Colour white(241, 246, 250);
}

WaveBombzEndlessSmileAudioProcessorEditor::WaveBombzEndlessSmileAudioProcessorEditor(WaveBombzEndlessSmileAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setResizable(true, true);
    setResizeLimits(620, 500, 1200, 900);
    setSize(900, 650);

    title.setText("WAVEBOMBZ", juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centredLeft);
    title.setColour(juce::Label::textColourId, white);
    title.setFont(juce::Font(juce::FontOptions(23.0f, juce::Font::bold)));
    addAndMakeVisible(title);

    subtitle.setText("ENDLESS SMILE  /  INTENSITY MULTI-EFFECT", juce::dontSendNotification);
    subtitle.setJustificationType(juce::Justification::centredLeft);
    subtitle.setColour(juce::Label::textColourId, teal);
    subtitle.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
    addAndMakeVisible(subtitle);

    intensity.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    intensity.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 24);
    intensity.setRange(0.0, 1.0, 0.001);
    intensity.setColour(juce::Slider::rotarySliderFillColourId, teal);
    intensity.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(54, 68, 81));
    intensity.setColour(juce::Slider::thumbColourId, white);
    intensity.setColour(juce::Slider::textBoxTextColourId, white);
    intensity.setColour(juce::Slider::textBoxBackgroundColourId, panelLight);
    intensity.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    intensity.textFromValueFunction = [](double value)
    {
        return juce::String(juce::roundToInt(value * 100.0)) + " %";
    };
    intensity.valueFromTextFunction = [](const juce::String& text)
    {
        return text.getDoubleValue() / 100.0;
    };
    addAndMakeVisible(intensity);

    character.addItem("CLASSIC", 1);
    character.addItem("MASSIVE", 2);
    character.addItem("TECH", 3);
    character.addItem("ATMOSPHERE", 4);
    character.setColour(juce::ComboBox::backgroundColourId, panelLight);
    character.setColour(juce::ComboBox::textColourId, white);
    character.setColour(juce::ComboBox::outlineColourId, juce::Colour(57, 77, 91));
    character.setColour(juce::ComboBox::arrowColourId, teal);
    addAndMakeVisible(character);

    intensityLabel.setText("INTENSITY", juce::dontSendNotification);
    intensityLabel.setJustificationType(juce::Justification::centred);
    intensityLabel.setColour(juce::Label::textColourId, teal);
    intensityLabel.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    addAndMakeVisible(intensityLabel);

    characterLabel.setText("SELECT CHARACTER", juce::dontSendNotification);
    characterLabel.setJustificationType(juce::Justification::centredLeft);
    characterLabel.setColour(juce::Label::textColourId, muted);
    characterLabel.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    addAndMakeVisible(characterLabel);

    intensityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "INTENSITY", intensity);
    characterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.parameters, "CHARACTER", character);
}

WaveBombzEndlessSmileAudioProcessorEditor::~WaveBombzEndlessSmileAudioProcessorEditor() = default;

void WaveBombzEndlessSmileAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(background);
    auto bounds = getLocalBounds().toFloat().reduced(12.0f);

    g.setColour(panel);
    g.fillRoundedRectangle(bounds, 16.0f);
    g.setColour(juce::Colour(43, 58, 71));
    g.drawRoundedRectangle(bounds, 16.0f, 1.0f);

    // Header accent and abstract energy-wave motif.
    g.setColour(teal);
    g.fillRoundedRectangle(juce::Rectangle<float>(bounds.getX() + 18.0f, bounds.getY() + 18.0f, 4.0f, 42.0f), 2.0f);
    auto motif = juce::Rectangle<float>(bounds.getRight() - 150.0f, bounds.getY() + 18.0f, 115.0f, 48.0f);
    juce::Path wave;
    wave.startNewSubPath(motif.getX(), motif.getCentreY());
    for (int i = 1; i <= 40; ++i)
    {
        const auto x = motif.getX() + motif.getWidth() * static_cast<float>(i) / 40.0f;
        const auto phase = static_cast<float>(i) * 0.48f;
        const auto envelope = std::sin(juce::MathConstants<float>::pi * static_cast<float>(i) / 40.0f);
        wave.lineTo(x, motif.getCentreY() + std::sin(phase) * envelope * motif.getHeight() * 0.43f);
    }
    g.setColour(teal.withAlpha(0.72f));
    g.strokePath(wave, juce::PathStrokeType(1.7f));

    const auto content = bounds.reduced(20.0f);
    const auto top = content.getY() + 68.0f;
    const auto mainH = content.getHeight() * 0.57f;
    const auto leftW = content.getWidth() * 0.25f;
    const auto centreW = content.getWidth() * 0.43f;
    auto preset = juce::Rectangle<float>(content.getX(), top, leftW - 10.0f, mainH);
    auto hero = juce::Rectangle<float>(preset.getRight() + 10.0f, top, centreW, mainH);
    auto side = juce::Rectangle<float>(hero.getRight() + 10.0f, top, content.getRight() - hero.getRight() - 10.0f, mainH);

    auto drawPanel = [&g](juce::Rectangle<float> r, const juce::String& heading)
    {
        g.setColour(panelLight.withAlpha(0.85f));
        g.fillRoundedRectangle(r, 10.0f);
        g.setColour(juce::Colour(48, 64, 79));
        g.drawRoundedRectangle(r, 10.0f, 1.0f);
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
        g.drawText(heading, r.withTrimmedLeft(12.0f).withTrimmedTop(10.0f).withHeight(18.0f), juce::Justification::centredLeft);
    };

    drawPanel(preset, "CHARACTER");
    const juce::StringArray names { "CLASSIC", "MASSIVE", "TECH", "ATMOSPHERE" };
    for (int i = 0; i < names.size(); ++i)
    {
        auto tile = preset.reduced(10.0f).withY(preset.getY() + 38.0f + static_cast<float>(i) * (preset.getHeight() - 48.0f) / 4.0f)
                    .withHeight((preset.getHeight() - 54.0f) / 4.0f);
        g.setColour(i == character.getSelectedId() - 1 ? teal.withAlpha(0.14f) : panel);
        g.fillRoundedRectangle(tile, 6.0f);
        g.setColour(i == character.getSelectedId() - 1 ? teal : juce::Colour(58, 73, 87));
        g.drawRoundedRectangle(tile, 6.0f, i == character.getSelectedId() - 1 ? 1.5f : 0.8f);
        g.setColour(i == character.getSelectedId() - 1 ? white : muted);
        g.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
        g.drawText(names[i], tile, juce::Justification::centred);
    }

    drawPanel(hero, "MASTER EFFECT");
    g.setColour(white);
    g.setFont(juce::Font(juce::FontOptions(25.0f, juce::Font::bold)));
    g.drawText("ENDLESS", hero.reduced(12.0f).withY(hero.getY() + 35.0f).withHeight(34.0f), juce::Justification::centred);
    g.setColour(teal);
    g.setFont(juce::Font(juce::FontOptions(22.0f, juce::Font::bold)));
    g.drawText("SMILE", hero.reduced(12.0f).withY(hero.getY() + 65.0f).withHeight(30.0f), juce::Justification::centred);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.drawText("BUILD THE MOMENT.", hero.reduced(10.0f).withY(hero.getBottom() - 28.0f).withHeight(16.0f), juce::Justification::centred);

    drawPanel(side, "SIGNAL FLOW");
    const juce::StringArray stages { "FILTER", "DELAY", "REVERB", "SATURATION" };
    for (int i = 0; i < stages.size(); ++i)
    {
        auto y = side.getY() + 48.0f + static_cast<float>(i) * (side.getHeight() - 64.0f) / 4.0f;
        g.setColour(teal.withAlpha(0.8f));
        g.fillEllipse(side.getX() + 14.0f, y + 2.0f, 6.0f, 6.0f);
        g.setColour(white);
        g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
        g.drawText(stages[i], side.getX() + 28.0f, y - 3.0f, side.getWidth() - 36.0f, 16.0f, juce::Justification::centredLeft);
        if (i < stages.size() - 1)
        {
            g.setColour(juce::Colour(58, 76, 91));
            g.drawLine(side.getX() + 17.0f, y + 12.0f, side.getX() + 17.0f, y + (side.getHeight() - 64.0f) / 4.0f + 2.0f, 1.0f);
        }
    }

    const auto modulesY = top + mainH + 14.0f;
    auto modules = juce::Rectangle<float>(content.getX(), modulesY, content.getWidth(), content.getBottom() - modulesY);
    const juce::StringArray moduleNames { "HIGH-PASS FILTER", "DELAY", "REVERB", "SATURATION" };
    const juce::StringArray moduleDetails { "24 dB / OCT", "DAW SYNC  •  3/4", "SPACE & TAIL", "SOFT CLIP" };
    const auto gap = 10.0f;
    const auto cardW = (modules.getWidth() - gap * 3.0f) / 4.0f;
    for (int i = 0; i < moduleNames.size(); ++i)
    {
        auto card = juce::Rectangle<float>(modules.getX() + static_cast<float>(i) * (cardW + gap), modules.getY(), cardW, modules.getHeight());
        g.setColour(panelLight);
        g.fillRoundedRectangle(card, 8.0f);
        g.setColour(juce::Colour(47, 64, 79));
        g.drawRoundedRectangle(card, 8.0f, 1.0f);
        g.setColour(teal);
        g.fillRoundedRectangle(card.getX() + 12.0f, card.getY() + 12.0f, 24.0f, 3.0f, 1.5f);
        g.setColour(white);
        g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
        g.drawText(moduleNames[i], card.reduced(10.0f).withY(card.getY() + 24.0f).withHeight(20.0f), juce::Justification::centredLeft, true);
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText(moduleDetails[i], card.reduced(10.0f).withY(card.getY() + 47.0f).withHeight(18.0f), juce::Justification::centredLeft, true);
    }
}

void WaveBombzEndlessSmileAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(30);
    auto header = area.removeFromTop(58);
    title.setBounds(header.removeFromTop(31).withTrimmedLeft(12));
    subtitle.setBounds(header.withTrimmedLeft(12));

    area.removeFromTop(82);
    auto top = area.removeFromTop(static_cast<int>(area.getHeight() * 0.70f));
    const auto leftW = static_cast<int>(top.getWidth() * 0.25f);
    const auto centreW = static_cast<int>(top.getWidth() * 0.43f);
    top.removeFromLeft(leftW + 10);
    top.removeFromRight(static_cast<int>(top.getWidth() - centreW));
    auto knobArea = top.reduced(12, 24);
    intensity.setBounds(knobArea.withSizeKeepingCentre(juce::jmin(220, knobArea.getWidth()), juce::jmin(220, knobArea.getHeight() - 28)));
    intensityLabel.setBounds(intensity.getX(), intensity.getBottom() - 4, intensity.getWidth(), 24);

    auto footer = area.removeFromBottom(34);
    characterLabel.setBounds(footer.removeFromLeft(125));
    character.setBounds(footer.removeFromLeft(230).withHeight(32));
}
