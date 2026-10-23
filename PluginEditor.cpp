#include "PluginEditor.h"

using namespace PinkyColours;

// ======================= LookAndFeel =======================
PinkyLookAndFeel::PinkyLookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, juce::Colours::white);
    setColour (juce::ComboBox::outlineColourId,    pink);
    setColour (juce::ComboBox::textColourId,       deepPink);
    setColour (juce::ComboBox::arrowColourId,      pink);
    setColour (juce::PopupMenu::backgroundColourId,            juce::Colours::white);
    setColour (juce::PopupMenu::textColourId,                  deepPink);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, pink);
    setColour (juce::PopupMenu::highlightedTextColourId,       juce::Colours::white);
    setColour (juce::Label::textColourId,                      text);
    setColour (juce::Slider::textBoxTextColourId,              deepPink);
    setColour (juce::Slider::textBoxOutlineColourId,           juce::Colours::transparentBlack);
}

void PinkyLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float startAngle, float endAngle,
                                         juce::Slider&)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (6.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    // мягкая тень
    g.setColour (pink.withAlpha (0.18f));
    g.fillEllipse (centre.x - radius, centre.y - radius + 3.0f, radius * 2.0f, radius * 2.0f);

    // тело ручки
    g.setGradientFill (juce::ColourGradient (juce::Colours::white, centre.x, centre.y - radius,
                                             softPink.brighter (0.2f), centre.x, centre.y + radius, false));
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    // дорожка и значение
    const float arcR = radius - 3.0f;
    juce::Path track, value;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);

    g.setColour (softPink);
    g.strokePath (track, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (deepPink);
    g.strokePath (value, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // стрелка
    juce::Path pointer;
    const float pl = radius * 0.55f;
    pointer.addRoundedRectangle (-1.8f, -arcR + 6.0f, 3.6f, pl * 0.6f, 1.5f);
    g.setColour (deepPink);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

// ======================= LfoView =======================
LfoView::LfoView (PinkyLiteAudioProcessor& p) : proc (p)
{
    startTimerHz (30);
}

void LfoView::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    g.setColour (juce::Colours::white);
    g.fillRoundedRectangle (b, 14.0f);
    g.setColour (softPink);
    g.drawRoundedRectangle (b.reduced (0.5f), 14.0f, 1.5f);

    const int   shape = (int) proc.apvts.getRawParameterValue ("shape")->load();
    const float depth = proc.apvts.getRawParameterValue ("volDepth")->load();

    auto area = b.reduced (14.0f, 16.0f);
    juce::Path curve;
    const int n = 200;
    for (int i = 0; i <= n; ++i)
    {
        const float p = (float) i / n;
        const float v = 1.0f - depth * (1.0f - PinkyLiteAudioProcessor::lfoShape (shape, p));
        const float px = area.getX() + p * area.getWidth();
        const float py = area.getBottom() - v * area.getHeight();
        if (i == 0) curve.startNewSubPath (px, py); else curve.lineTo (px, py);
    }

    juce::Path fill (curve);
    fill.lineTo (area.getRight(), area.getBottom());
    fill.lineTo (area.getX(), area.getBottom());
    fill.closeSubPath();

    g.setGradientFill (juce::ColourGradient (pink.withAlpha (0.55f), 0.0f, area.getY(),
                                             softPink.withAlpha (0.10f), 0.0f, area.getBottom(), false));
    g.fillPath (fill);

    g.setColour (deepPink);
    g.strokePath (curve, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // бегунок по текущей фазе
    const float ph = proc.uiPhase.load();
    const float lx = area.getX() + ph * area.getWidth();
    g.setColour (pink.withAlpha (0.8f));
    g.drawLine (lx, area.getY(), lx, area.getBottom(), 2.0f);
}

// ======================= Editor =======================
namespace
{
    constexpr int cellW = 80;
    constexpr int startX = 20;
    constexpr int groupY = 238;
    constexpr int groupH = 146;

    struct Group { const char* name; int first; int count; };
    const Group groups[] = { { "VOLUME", 0, 1 }, { "FILTER", 1, 3 }, { "DRIVE", 4, 1 },
                             { "CRUSH", 5, 1 },  { "PAN", 6, 1 },    { "OUTPUT", 7, 2 } };
}

PinkyLiteAudioProcessorEditor::PinkyLiteAudioProcessorEditor (PinkyLiteAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p), lfoView (p)
{
    setLookAndFeel (&laf);
    setSize (760, 410);

    addAndMakeVisible (lfoView);

    for (auto* c : { &rateBox, &shapeBox })
        addAndMakeVisible (c);

    rateBox.addItemList (PinkyLiteAudioProcessor::rateNames(), 1);
    shapeBox.addItemList (PinkyLiteAudioProcessor::shapeNames(), 1);
    rateAtt  = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "rate",  rateBox);
    shapeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "shape", shapeBox);

    for (auto* l : { &rateLabel, &shapeLabel })
    {
        addAndMakeVisible (l);
        l->setJustificationType (juce::Justification::centredLeft);
    }
    rateLabel.setText ("RATE", juce::dontSendNotification);
    shapeLabel.setText ("SHAPE", juce::dontSendNotification);

    addKnob (depth,  "volDepth", "Depth");
    addKnob (cutoff, "cutoff",   "Cutoff");
    addKnob (reso,   "reso",     "Reso");
    addKnob (cutMod, "cutMod",   "LFO Mod");
    addKnob (drive,  "drive",    "Drive");
    addKnob (crush,  "crush",    "Crush");
    addKnob (pan,    "pan",      "Width");
    addKnob (mix,    "mix",      "Mix");
    addKnob (out,    "out",      "Gain");

    // красивый текст значений
    auto percent = [] (juce::Slider& s)
    {
        s.textFromValueFunction = [] (double v) { return juce::String (juce::roundToInt (v * 100.0)) + " %"; };
        s.updateText();
    };
    for (auto* s : { &depth.slider, &cutMod.slider, &drive.slider, &crush.slider, &pan.slider, &mix.slider })
        percent (*s);

    cutoff.slider.textFromValueFunction = [] (double v)
    {
        return v >= 1000.0 ? juce::String (v / 1000.0, 1) + " kHz" : juce::String (juce::roundToInt (v)) + " Hz";
    };
    cutoff.slider.updateText();

    reso.slider.textFromValueFunction = [] (double v) { return juce::String (v, 1) + " Q"; };
    reso.slider.updateText();

    out.slider.textFromValueFunction = [] (double v) { return juce::String (v, 1) + " dB"; };
    out.slider.updateText();
}

PinkyLiteAudioProcessorEditor::~PinkyLiteAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void PinkyLiteAudioProcessorEditor::addKnob (Knob& k, const juce::String& paramId, const juce::String& name)
{
    k.slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
    k.slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                                  juce::MathConstants<float>::pi * 2.8f, true);
    k.slider.setDoubleClickReturnValue (true, proc.apvts.getParameter (paramId)->convertFrom0to1 (
                                                  proc.apvts.getParameter (paramId)->getDefaultValue()));
    addAndMakeVisible (k.slider);

    k.label.setText (name, juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    k.label.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    addAndMakeVisible (k.label);

    k.attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, paramId, k.slider);
}

void PinkyLiteAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (bgTop, 0.0f, 0.0f, bgBottom, 0.0f, (float) getHeight(), false));
    g.fillAll();

    // заголовок
    g.setColour (deepPink);
    g.setFont (juce::FontOptions (30.0f, juce::Font::bold));
    g.drawText ("Pinky", 22, 10, 110, 40, juce::Justification::centredLeft);
    g.setColour (pink);
    g.setFont (juce::FontOptions (30.0f, juce::Font::plain));
    g.drawText ("Lite", 100, 10, 80, 40, juce::Justification::centredLeft);

    g.setColour (text.withAlpha (0.6f));
    g.setFont (juce::FontOptions (12.0f));
    g.drawText ("volume \xc2\xb7 filter \xc2\xb7 drive \xc2\xb7 crush \xc2\xb7 pan", getWidth() - 320, 18, 300, 24,
                juce::Justification::centredRight);

    // панели секций
    for (auto& grp : groups)
    {
        auto r = juce::Rectangle<float> ((float) (startX + grp.first * cellW + 2), (float) groupY,
                                         (float) (grp.count * cellW - 4), (float) groupH);
        g.setColour (panel);
        g.fillRoundedRectangle (r, 14.0f);
        g.setColour (softPink);
        g.drawRoundedRectangle (r.reduced (0.5f), 14.0f, 1.2f);

        g.setColour (pink);
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (grp.name, r.withHeight (22.0f).toNearestInt(), juce::Justification::centred);
    }
}

void PinkyLiteAudioProcessorEditor::resized()
{
    lfoView.setBounds (20, 64, 500, 160);

    rateLabel.setBounds  (544, 70, 190, 18);
    rateBox.setBounds    (544, 90, 190, 30);
    shapeLabel.setBounds (544, 136, 190, 18);
    shapeBox.setBounds   (544, 156, 190, 30);

    for (int i = 0; i < 9; ++i)
    {
        auto* k = knobs[i];
        const int x = startX + i * cellW;
        k->label.setBounds  (x, groupY + 22, cellW, 16);
        k->slider.setBounds (x + 2, groupY + 38, cellW - 4, groupH - 42);
    }
}
