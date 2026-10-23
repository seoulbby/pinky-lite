#include "PluginEditor.h"

// Pinky Lite - made by seoulbby

// ======================= Темы =======================
const Theme& getTheme (int index)
{
    static const Theme themes[] =
    {
        // Pinky
        { juce::Colour (0xffffffff), juce::Colour (0xffffe6f0), juce::Colour (0xffff6fa8), juce::Colour (0xffe0457f),
          juce::Colour (0xffffc9de), juce::Colour (0xfffff2f7), juce::Colour (0xff9c2f5e) },
        // Bubblegum
        { juce::Colour (0xfffffbff), juce::Colour (0xfff3e6ff), juce::Colour (0xffd77bff), juce::Colour (0xffa73fe0),
          juce::Colour (0xffecc9ff), juce::Colour (0xfff8f0ff), juce::Colour (0xff6b2a9c) },
        // Peach
        { juce::Colour (0xfffffdfa), juce::Colour (0xffffeadb), juce::Colour (0xffff9a6e), juce::Colour (0xffe06a3a),
          juce::Colour (0xffffd3bd), juce::Colour (0xfffff4ec), juce::Colour (0xff9c4a2f) }
    };
    return themes[juce::jlimit (0, 2, index)];
}

// ======================= LookAndFeel =======================
PinkyLookAndFeel::PinkyLookAndFeel()
{
    setTheme (0);
}

void PinkyLookAndFeel::setTheme (int index)
{
    theme = getTheme (index);

    setColour (juce::ComboBox::backgroundColourId, juce::Colours::white);
    setColour (juce::ComboBox::outlineColourId,    theme.pink);
    setColour (juce::ComboBox::textColourId,       theme.deepPink);
    setColour (juce::ComboBox::arrowColourId,      theme.pink);
    setColour (juce::PopupMenu::backgroundColourId,            juce::Colours::white);
    setColour (juce::PopupMenu::textColourId,                  theme.deepPink);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, theme.pink);
    setColour (juce::PopupMenu::highlightedTextColourId,       juce::Colours::white);
    setColour (juce::Label::textColourId,                      theme.text);
    setColour (juce::Slider::textBoxTextColourId,              theme.deepPink);
    setColour (juce::Slider::textBoxOutlineColourId,           juce::Colours::transparentBlack);
    setColour (juce::TextButton::textColourOffId,              juce::Colours::white);
    setColour (juce::TextButton::textColourOnId,               juce::Colours::white);
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
    g.setColour (theme.pink.withAlpha (0.18f));
    g.fillEllipse (centre.x - radius, centre.y - radius + 3.0f, radius * 2.0f, radius * 2.0f);

    // тело ручки
    g.setGradientFill (juce::ColourGradient (juce::Colours::white, centre.x, centre.y - radius,
                                             theme.softPink.brighter (0.2f), centre.x, centre.y + radius, false));
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    // дорожка и значение
    const float arcR = radius - 3.0f;
    juce::Path track, value;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);

    g.setColour (theme.softPink);
    g.strokePath (track, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (theme.deepPink);
    g.strokePath (value, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // стрелка
    juce::Path pointer;
    const float pl = radius * 0.55f;
    pointer.addRoundedRectangle (-1.8f, -arcR + 6.0f, 3.6f, pl * 0.6f, 1.5f);
    g.setColour (theme.deepPink);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

void PinkyLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&,
                                             bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    auto c = theme.pink;
    if (down)             c = theme.deepPink;
    else if (highlighted) c = theme.pink.brighter (0.15f);

    g.setColour (c);
    g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
}

void PinkyLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    auto sw = b.getLocalBounds().toFloat().reduced (2.0f);
    const bool on = b.getToggleState();

    g.setColour (on ? theme.pink : theme.softPink);
    g.fillRoundedRectangle (sw, sw.getHeight() * 0.5f);

    if (highlighted)
    {
        g.setColour (theme.deepPink.withAlpha (0.25f));
        g.drawRoundedRectangle (sw, sw.getHeight() * 0.5f, 1.5f);
    }

    const float d = sw.getHeight() - 6.0f;
    const float kx = on ? sw.getRight() - d - 3.0f : sw.getX() + 3.0f;
    g.setColour (juce::Colours::white);
    g.fillEllipse (kx, sw.getY() + 3.0f, d, d);
}

// ======================= LfoView =======================
LfoView::LfoView (PinkyLiteAudioProcessor& p, const Theme& t) : proc (p), theme (t)
{
    startTimerHz (30);
}

void LfoView::timerCallback()
{
    const float s = proc.apvts.getRawParameterValue ("shape")->load();
    const float d = proc.apvts.getRawParameterValue ("volDepth")->load();
    const bool changed = (s != lastShape) || (d != lastDepth);
    lastShape = s;
    lastDepth = d;

    if (animate || changed)
        repaint();
}

void LfoView::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    g.setColour (juce::Colours::white);
    g.fillRoundedRectangle (b, 14.0f);
    g.setColour (theme.softPink);
    g.drawRoundedRectangle (b.reduced (0.5f), 14.0f, 1.5f);

    const int   shape = (int) proc.apvts.getRawParameterValue ("shape")->load();
    const float depth = proc.apvts.getRawParameterValue ("volDepth")->load();

    auto area = b.reduced (14.0f, 16.0f);
    juce::Path curve;
    const int n = 240;
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

    g.setGradientFill (juce::ColourGradient (theme.pink.withAlpha (0.55f), 0.0f, area.getY(),
                                             theme.softPink.withAlpha (0.10f), 0.0f, area.getBottom(), false));
    g.fillPath (fill);

    g.setColour (theme.deepPink);
    g.strokePath (curve, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (animate)
    {
        const float ph = proc.uiPhase.load();
        const float lx = area.getX() + ph * area.getWidth();
        g.setColour (theme.pink.withAlpha (0.8f));
        g.drawLine (lx, area.getY(), lx, area.getBottom(), 2.0f);
    }
}

// ======================= SettingsPanel =======================
SettingsPanel::SettingsPanel (PinkyLiteAudioProcessor& p, const Theme& t, std::function<void()> onClose)
    : proc (p), theme (t), closeFn (std::move (onClose))
{
    osBox.addItemList (PinkyLiteAudioProcessor::osNames(), 1);
    themeBox.addItemList (PinkyLiteAudioProcessor::themeNames(), 1);
    osAtt    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "os",    osBox);
    themeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "theme", themeBox);
    safeAtt  = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>   (proc.apvts, "safe",  safeBtn);
    animAtt  = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>   (proc.apvts, "anim",  animBtn);

    for (auto* c : { (juce::Component*) &osBox, (juce::Component*) &themeBox, (juce::Component*) &safeBtn,
                     (juce::Component*) &animBtn, (juce::Component*) &resetBtn, (juce::Component*) &closeBtn })
        addAndMakeVisible (c);

    closeBtn.onClick = [this] { if (closeFn) closeFn(); };

    resetBtn.onClick = [this]
    {
        // сбрасываем только звук, а не сами настройки плагина
        static const juce::StringArray keep { "os", "safe", "theme", "anim" };
        for (auto* prm : proc.getParameters())
            if (auto* r = dynamic_cast<juce::RangedAudioParameter*> (prm))
                if (! keep.contains (r->paramID))
                {
                    r->beginChangeGesture();
                    r->setValueNotifyingHost (r->getDefaultValue());
                    r->endChangeGesture();
                }
    };
}

juce::Rectangle<int> SettingsPanel::card() const
{
    return getLocalBounds().withSizeKeepingCentre (560, 410);
}

void SettingsPanel::resized()
{
    auto c = card();
    const int y0 = c.getY() + 84;
    const int rowH = 50;
    const int right = c.getRight() - 28;

    closeBtn.setBounds (right - 84, c.getY() + 22, 84, 30);

    osBox.setBounds    (right - 170, y0 + 0 * rowH + 4, 170, 30);
    safeBtn.setBounds  (right - 54,  y0 + 1 * rowH + 6, 54, 28);
    themeBox.setBounds (right - 170, y0 + 2 * rowH + 4, 170, 30);
    animBtn.setBounds  (right - 54,  y0 + 3 * rowH + 6, 54, 28);

    resetBtn.setBounds (c.getX() + 28, y0 + 4 * rowH + 6, 140, 32);
}

void SettingsPanel::mouseDown (const juce::MouseEvent& e)
{
    if (! card().contains (e.getPosition()) && closeFn)
        closeFn();
}

void SettingsPanel::paint (juce::Graphics& g)
{
    g.fillAll (theme.bgTop.withAlpha (0.82f));

    auto c = card();
    juce::DropShadow (juce::Colours::black.withAlpha (0.16f), 28, { 0, 8 }).drawForRectangle (g, c);

    g.setColour (juce::Colours::white);
    g.fillRoundedRectangle (c.toFloat(), 20.0f);
    g.setColour (theme.softPink);
    g.drawRoundedRectangle (c.toFloat().reduced (0.5f), 20.0f, 1.5f);

    g.setColour (theme.deepPink);
    g.setFont (juce::FontOptions (24.0f, juce::Font::bold));
    g.drawText ("Settings", c.getX() + 28, c.getY() + 20, 260, 34, juce::Justification::centredLeft);

    struct Row { const char* title; const char* hint; };
    const Row rows[] =
    {
        { "Oversampling",   "Cleaner distortion, adds a bit of latency" },
        { "Safe limiter",   "Soft-limits the output so nothing blows up" },
        { "Theme",          "Pick your colours" },
        { "Animations",     "Moving playhead on the LFO display" }
    };

    const int y0 = c.getY() + 84;
    for (int i = 0; i < 4; ++i)
    {
        const int y = y0 + i * 50;
        g.setColour (theme.text);
        g.setFont (juce::FontOptions (15.0f, juce::Font::bold));
        g.drawText (rows[i].title, c.getX() + 28, y, 260, 20, juce::Justification::centredLeft);
        g.setColour (theme.text.withAlpha (0.6f));
        g.setFont (juce::FontOptions (11.5f));
        g.drawText (rows[i].hint, c.getX() + 28, y + 20, 280, 16, juce::Justification::centredLeft);
    }

    g.setColour (theme.text.withAlpha (0.6f));
    g.setFont (juce::FontOptions (11.5f));
    g.drawText ("Sets all effect knobs back to default", c.getX() + 182, y0 + 4 * 50 + 6, 300, 32,
                juce::Justification::centredLeft);

    // About
    const int ay = c.getBottom() - 70;
    g.setColour (theme.softPink);
    g.drawLine ((float) c.getX() + 28.0f, (float) ay - 8.0f, (float) c.getRight() - 28.0f, (float) ay - 8.0f, 1.0f);

    g.setColour (theme.deepPink);
    g.setFont (juce::FontOptions (16.0f, juce::Font::bold));
    g.drawText ("Pinky Lite  v0.2", c.getX() + 28, ay, 300, 22, juce::Justification::centredLeft);
    g.setColour (theme.pink);
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("made by seoulbby", c.getX() + 28, ay + 24, 300, 20, juce::Justification::centredLeft);
}

// ======================= Editor =======================
namespace
{
    constexpr int cellW = 70;
    constexpr int startX = 20;
    constexpr int groupY = 250;
    constexpr int groupH = 152;

    struct Group { const char* name; int first; int count; };
    const Group groups[] = { { "VOLUME", 0, 1 }, { "FILTER", 1, 3 }, { "DISTORT", 4, 4 },
                             { "CRUSH", 8, 1 },  { "RING", 9, 2 },   { "PAN", 11, 1 },
                             { "OUTPUT", 12, 2 } };
}

PinkyLiteAudioProcessorEditor::PinkyLiteAudioProcessorEditor (PinkyLiteAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p), lfoView (p, laf.theme),
      settings (p, laf.theme, [this] { settings.setVisible (false); })
{
    themeIdx = juce::jlimit (0, 2, (int) proc.apvts.getRawParameterValue ("theme")->load());
    laf.setTheme (themeIdx);
    setLookAndFeel (&laf);
    setSize (1020, 450);

    addAndMakeVisible (lfoView);
    lfoView.animate = proc.apvts.getRawParameterValue ("anim")->load() > 0.5f;

    addCombo (rateBox,  rateLabel,  "RATE",        PinkyLiteAudioProcessor::rateNames(),   "rate",     rateAtt);
    addCombo (shapeBox, shapeLabel, "SHAPE",       PinkyLiteAudioProcessor::shapeNames(),  "shape",    shapeAtt);
    addCombo (filtBox,  filtLabel,  "FILTER TYPE", PinkyLiteAudioProcessor::filterNames(), "filtType", filtAtt);
    addCombo (distBox,  distLabel,  "DIST TYPE",   PinkyLiteAudioProcessor::distNames(),   "distType", distAtt);

    addKnob (depth,    "volDepth", "Depth");
    addKnob (cutoff,   "cutoff",   "Cutoff");
    addKnob (reso,     "reso",     "Reso");
    addKnob (cutMod,   "cutMod",   "LFO Mod");
    addKnob (drive,    "drive",    "Drive");
    addKnob (bias,     "bias",     "Bias");
    addKnob (tone,     "tone",     "Tone");
    addKnob (drvMod,   "drvMod",   "LFO Mod");
    addKnob (crush,    "crush",    "Crush");
    addKnob (ringAmt,  "ringAmt",  "Amount");
    addKnob (ringFreq, "ringFreq", "Freq");
    addKnob (pan,      "pan",      "Width");
    addKnob (mix,      "mix",      "Mix");
    addKnob (out,      "out",      "Gain");

    // красивый текст значений
    auto percent = [] (juce::Slider& s)
    {
        s.textFromValueFunction = [] (double v) { return juce::String (juce::roundToInt (v * 100.0)) + " %"; };
        s.updateText();
    };
    for (auto* s : { &depth.slider, &cutMod.slider, &drive.slider, &tone.slider, &drvMod.slider,
                     &crush.slider, &ringAmt.slider, &pan.slider, &mix.slider })
        percent (*s);

    bias.slider.textFromValueFunction = [] (double v)
    {
        const int pc = juce::roundToInt (v * 100.0);
        return (pc > 0 ? juce::String ("+") : juce::String()) + juce::String (pc) + " %";
    };
    bias.slider.updateText();

    auto hz = [] (double v)
    {
        return v >= 1000.0 ? juce::String (v / 1000.0, 1) + " kHz" : juce::String (juce::roundToInt (v)) + " Hz";
    };
    cutoff.slider.textFromValueFunction   = hz;   cutoff.slider.updateText();
    ringFreq.slider.textFromValueFunction = hz;   ringFreq.slider.updateText();

    reso.slider.textFromValueFunction = [] (double v) { return juce::String (v, 1) + " Q"; };
    reso.slider.updateText();

    out.slider.textFromValueFunction = [] (double v) { return juce::String (v, 1) + " dB"; };
    out.slider.updateText();

    addAndMakeVisible (settingsBtn);
    settingsBtn.onClick = [this]
    {
        settings.setVisible (true);
        settings.toFront (true);
    };

    addChildComponent (settings);

    startTimerHz (10);
}

PinkyLiteAudioProcessorEditor::~PinkyLiteAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void PinkyLiteAudioProcessorEditor::timerCallback()
{
    const int t = juce::jlimit (0, 2, (int) proc.apvts.getRawParameterValue ("theme")->load());
    if (t != themeIdx)
    {
        themeIdx = t;
        laf.setTheme (t);
        sendLookAndFeelChange();
        repaint();
    }

    lfoView.animate = proc.apvts.getRawParameterValue ("anim")->load() > 0.5f;
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

void PinkyLiteAudioProcessorEditor::addCombo (juce::ComboBox& box, juce::Label& label, const juce::String& title,
                                              const juce::StringArray& items, const juce::String& paramId,
                                              std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>& att)
{
    box.addItemList (items, 1);
    addAndMakeVisible (box);

    label.setText (title, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centredLeft);
    label.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    addAndMakeVisible (label);

    att = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, paramId, box);
}

void PinkyLiteAudioProcessorEditor::paint (juce::Graphics& g)
{
    const auto& th = laf.theme;

    g.setGradientFill (juce::ColourGradient (th.bgTop, 0.0f, 0.0f, th.bgBottom, 0.0f, (float) getHeight(), false));
    g.fillAll();

    // заголовок
    g.setColour (th.deepPink);
    g.setFont (juce::FontOptions (32.0f, juce::Font::bold));
    g.drawText ("Pinky", 22, 4, 120, 40, juce::Justification::centredLeft);
    g.setColour (th.pink);
    g.setFont (juce::FontOptions (32.0f, juce::Font::plain));
    g.drawText ("Lite", 108, 4, 80, 40, juce::Justification::centredLeft);

    g.setColour (th.text.withAlpha (0.75f));
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("by seoulbby", 24, 40, 160, 18, juce::Justification::centredLeft);

    // подсказка
    g.setColour (th.text.withAlpha (0.5f));
    g.setFont (juce::FontOptions (11.5f));
    g.drawText ("tip: double-click any knob to reset it", 680, 204, 320, 20, juce::Justification::centredLeft);

    // панели секций
    for (auto& grp : groups)
    {
        auto r = juce::Rectangle<float> ((float) (startX + grp.first * cellW + 2), (float) groupY,
                                         (float) (grp.count * cellW - 4), (float) groupH);
        g.setColour (th.panel);
        g.fillRoundedRectangle (r, 14.0f);
        g.setColour (th.softPink);
        g.drawRoundedRectangle (r.reduced (0.5f), 14.0f, 1.2f);

        g.setColour (th.pink);
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (grp.name, r.withHeight (22.0f).toNearestInt(), juce::Justification::centred);
    }

    // подвал
    g.setColour (th.text.withAlpha (0.55f));
    g.setFont (juce::FontOptions (12.0f));
    g.drawText ("Pinky Lite v0.2  \xc2\xb7  made by seoulbby", 20, getHeight() - 32, getWidth() - 40, 22,
                juce::Justification::centred);
}

void PinkyLiteAudioProcessorEditor::resized()
{
    settingsBtn.setBounds (getWidth() - 20 - 110, 14, 110, 30);

    lfoView.setBounds (20, 68, 640, 166);

    const int x1 = 680, x2 = 850, w = 150;
    rateLabel.setBounds  (x1, 70,  w, 16);   rateBox.setBounds  (x1, 88,  w, 30);
    shapeLabel.setBounds (x2, 70,  w, 16);   shapeBox.setBounds (x2, 88,  w, 30);
    filtLabel.setBounds  (x1, 134, w, 16);   filtBox.setBounds  (x1, 152, w, 30);
    distLabel.setBounds  (x2, 134, w, 16);   distBox.setBounds  (x2, 152, w, 30);

    for (int i = 0; i < 14; ++i)
    {
        auto* k = knobs[i];
        const int x = startX + i * cellW;
        k->label.setBounds  (x, groupY + 22, cellW, 16);
        k->slider.setBounds (x + 2, groupY + 38, cellW - 4, groupH - 42);
    }

    settings.setBounds (getLocalBounds());
}
