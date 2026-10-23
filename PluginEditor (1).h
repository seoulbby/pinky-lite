#pragma once

#include "PluginProcessor.h"
#include <functional>

// Pinky Lite - made by seoulbby

// ---------- Цветовые темы ----------
struct Theme
{
    juce::Colour bgTop, bgBottom, pink, deepPink, softPink, panel, text;
};

const Theme& getTheme (int index);

class PinkyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PinkyLookAndFeel();

    void setTheme (int index);
    Theme theme;

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

// ---------- Визуализация LFO ----------
class LfoView : public juce::Component, private juce::Timer
{
public:
    LfoView (PinkyLiteAudioProcessor& p, const Theme& t);
    void paint (juce::Graphics&) override;

    bool animate = true;

private:
    void timerCallback() override;

    PinkyLiteAudioProcessor& proc;
    const Theme& theme;
    float lastShape = -1.0f, lastDepth = -1.0f;
};

// ---------- Окно настроек ----------
class SettingsPanel : public juce::Component
{
public:
    SettingsPanel (PinkyLiteAudioProcessor& p, const Theme& t, std::function<void()> onClose);

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    juce::Rectangle<int> card() const;

    PinkyLiteAudioProcessor& proc;
    const Theme& theme;
    std::function<void()> closeFn;

    juce::ComboBox osBox, themeBox;
    juce::ToggleButton safeBtn, animBtn;
    juce::TextButton resetBtn { "Reset sound" }, closeBtn { "Close" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osAtt, themeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> safeAtt, animAtt;
};

// ---------- Редактор ----------
class PinkyLiteAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PinkyLiteAudioProcessorEditor (PinkyLiteAudioProcessor&);
    ~PinkyLiteAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };

    void addKnob (Knob& k, const juce::String& paramId, const juce::String& name);
    void addCombo (juce::ComboBox& box, juce::Label& label, const juce::String& title,
                   const juce::StringArray& items, const juce::String& paramId,
                   std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>& att);
    void timerCallback() override;

    PinkyLiteAudioProcessor& proc;
    PinkyLookAndFeel laf;
    int themeIdx = 0;

    LfoView lfoView;

    juce::ComboBox rateBox, shapeBox, filtBox, distBox;
    juce::Label rateLabel, shapeLabel, filtLabel, distLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rateAtt, shapeAtt, filtAtt, distAtt;

    // порядок = порядок слева направо
    Knob depth, cutoff, reso, cutMod, drive, bias, tone, drvMod, crush, ringAmt, ringFreq, pan, mix, out;
    Knob* knobs[14] { &depth, &cutoff, &reso, &cutMod, &drive, &bias, &tone, &drvMod,
                      &crush, &ringAmt, &ringFreq, &pan, &mix, &out };

    juce::TextButton settingsBtn { "Settings" };
    SettingsPanel settings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PinkyLiteAudioProcessorEditor)
};
