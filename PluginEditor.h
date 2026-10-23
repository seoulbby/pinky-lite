#pragma once

#include "PluginProcessor.h"

// ---------- Бело-розовая тема ----------
namespace PinkyColours
{
    const juce::Colour bgTop    { 0xffffffff };
    const juce::Colour bgBottom { 0xffffe6f0 };
    const juce::Colour pink     { 0xffff6fa8 };
    const juce::Colour deepPink { 0xffe0457f };
    const juce::Colour softPink { 0xffffc9de };
    const juce::Colour panel    { 0xfffff2f7 };
    const juce::Colour text     { 0xff9c2f5e };
}

class PinkyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PinkyLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider&) override;
};

// ---------- Визуализация LFO ----------
class LfoView : public juce::Component, private juce::Timer
{
public:
    explicit LfoView (PinkyLiteAudioProcessor& p);
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override { repaint(); }
    PinkyLiteAudioProcessor& proc;
};

// ---------- Редактор ----------
class PinkyLiteAudioProcessorEditor : public juce::AudioProcessorEditor
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

    PinkyLiteAudioProcessor& proc;
    PinkyLookAndFeel laf;

    LfoView lfoView;
    juce::ComboBox rateBox, shapeBox;
    juce::Label rateLabel, shapeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rateAtt, shapeAtt;

    // порядок = порядок слева направо
    Knob depth, cutoff, reso, cutMod, drive, crush, pan, mix, out;
    Knob* knobs[9] { &depth, &cutoff, &reso, &cutMod, &drive, &crush, &pan, &mix, &out };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PinkyLiteAudioProcessorEditor)
};
