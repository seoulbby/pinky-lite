#include "PluginProcessor.h"
#include "PluginEditor.h"

// Pinky Lite - made by seoulbby

namespace
{
    // сколько четвертных нот в одном цикле LFO
    const double beatsPerCycle[] = { 4.0, 2.0, 1.0, 0.5, 0.25, 0.125 };
}

const juce::StringArray& PinkyLiteAudioProcessor::rateNames()
{
    static const juce::StringArray n { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32" };
    return n;
}

const juce::StringArray& PinkyLiteAudioProcessor::shapeNames()
{
    static const juce::StringArray n { "Sine", "Triangle", "Ramp", "Pulse", "Bounce" };
    return n;
}

const juce::StringArray& PinkyLiteAudioProcessor::filterNames()
{
    static const juce::StringArray n { "Low-pass", "High-pass", "Band-pass" };
    return n;
}

const juce::StringArray& PinkyLiteAudioProcessor::distNames()
{
    static const juce::StringArray n { "Soft", "Hard", "Fold", "Tube", "Fuzz", "Rectify" };
    return n;
}

const juce::StringArray& PinkyLiteAudioProcessor::osNames()
{
    static const juce::StringArray n { "Off", "2x", "4x" };
    return n;
}

const juce::StringArray& PinkyLiteAudioProcessor::themeNames()
{
    static const juce::StringArray n { "Pinky", "Bubblegum", "Peach" };
    return n;
}

float PinkyLiteAudioProcessor::lfoShape (int shape, float p)
{
    const float twoPi = juce::MathConstants<float>::twoPi;
    switch (shape)
    {
        case 0:  return 0.5f + 0.5f * std::cos (twoPi * p);                       // Sine
        case 1:  return p < 0.5f ? 1.0f - 2.0f * p : 2.0f * p - 1.0f;             // Triangle
        case 2:  return 1.0f - p;                                                 // Ramp down
        case 3:                                                                   // Pulse (мягкий)
        {
            const float edge = 0.04f;
            if (p < edge)                return p / edge;
            if (p < 0.5f)                return 1.0f;
            if (p < 0.5f + edge)         return 1.0f - (p - 0.5f) / edge;
            return 0.0f;
        }
        default:                                                                  // Bounce
        {
            const float q = std::fmod (p * 2.0f, 1.0f);
            return 1.0f - q * q;
        }
    }
}

// ---- формы искажения ----
float PinkyLiteAudioProcessor::shapeSample (int type, float u)
{
    switch (type)
    {
        case 0:  return std::tanh (u);                                   // Soft
        case 1:  return juce::jlimit (-1.0f, 1.0f, u);                   // Hard clip
        case 2:                                                          // Fold (треугольный wavefolder)
        {
            float f = u * 0.25f + 0.25f;
            f -= std::floor (f);
            return 1.0f - std::abs (4.0f * f - 2.0f);
        }
        case 3:  return std::tanh (u + 0.3f) - std::tanh (0.3f);         // Tube (асимметрия, чётные гармоники)
        case 4:                                                          // Fuzz (плюёт и гейтит)
        {
            const float t = std::tanh (u);
            const float s = std::sqrt (std::abs (t));
            return t < 0.0f ? -s : s;
        }
        default: return std::abs (std::tanh (u)) * 1.6f;                 // Rectify (октава вверх)
    }
}

float PinkyLiteAudioProcessor::softLimit (float y)
{
    const float t = 0.9f;
    const float a = std::abs (y);
    if (a <= t)
        return y;
    const float o = t + (1.0f - t) * std::tanh ((a - t) / (1.0f - t));
    return y < 0.0f ? -o : o;
}

juce::AudioProcessorValueTreeState::ParameterLayout PinkyLiteAudioProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;

    auto flt = [&] (const char* id, const char* name, NormalisableRange<float> r, float def)
    {
        l.add (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, r, def));
    };

    // Volume / LFO
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "rate", 1 },  "Rate",  rateNames(),  2));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "shape", 1 }, "Shape", shapeNames(), 0));
    flt ("volDepth", "Volume Depth", { 0.f, 1.f }, 0.7f);

    // Filter
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "filtType", 1 }, "Filter Type", filterNames(), 0));
    flt ("cutoff", "Cutoff",     { 20.f, 20000.f, 0.f, 0.3f }, 20000.f);
    flt ("reso",   "Resonance",  { 0.5f, 10.f, 0.f, 0.5f },    0.8f);
    flt ("cutMod", "Filter Mod", { 0.f, 1.f },                 0.0f);

    // Distortion
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "distType", 1 }, "Dist Type", distNames(), 0));
    flt ("drive",  "Drive",      { 0.f, 1.f },   0.0f);
    flt ("bias",   "Bias",       { -1.f, 1.f },  0.0f);
    flt ("tone",   "Tone",       { 0.f, 1.f },   1.0f);
    flt ("drvMod", "Drive Mod",  { 0.f, 1.f },   0.0f);

    // Crush / Ring / Pan
    flt ("crush",    "Crush",     { 0.f, 1.f }, 0.0f);
    flt ("ringAmt",  "Ring Amount", { 0.f, 1.f }, 0.0f);
    flt ("ringFreq", "Ring Freq", { 20.f, 4000.f, 0.f, 0.35f }, 440.f);
    flt ("pan",      "Auto Pan",  { 0.f, 1.f }, 0.0f);

    // Output
    flt ("mix", "Mix",    { 0.f, 1.f },     1.0f);
    flt ("out", "Output", { -24.f, 12.f },  0.0f);

    // Settings
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "os", 1 },    "Oversampling", osNames(),    1));
    l.add (std::make_unique<AudioParameterBool>   (ParameterID { "safe", 1 }, "Safe Limiter", true));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "theme", 1 }, "Theme",        themeNames(), 0));
    l.add (std::make_unique<AudioParameterBool>   (ParameterID { "anim", 1 }, "Animations",   true));

    return l;
}

PinkyLiteAudioProcessor::PinkyLiteAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
}

bool PinkyLiteAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return out == layouts.getMainInputChannelSet();
}

void PinkyLiteAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    maxBlock = juce::jmax (1, samplesPerBlock);

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlock, 2 };
    filter.prepare (spec);
    filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.reset();

    os2.initProcessing ((size_t) maxBlock);
    os4.initProcessing ((size_t) maxBlock);
    os2.reset();
    os4.reset();
    osLatency[0] = 0;
    osLatency[1] = (int) std::lround (os2.getLatencyInSamples());
    osLatency[2] = (int) std::lround (os4.getLatencyInSamples());
    appliedMode = -1;

    lfoBuf.assign ((size_t) maxBlock, 0.f);
    phaseBuf.assign ((size_t) maxBlock, 0.f);
    dryRing[0].assign ((size_t) ringSize, 0.f);
    dryRing[1].assign ((size_t) ringSize, 0.f);
    ringPos = 0;

    mixSm.reset (sampleRate, 0.02);
    gainSm.reset (sampleRate, 0.02);
    mixSm.setCurrentAndTargetValue (apvts.getRawParameterValue ("mix")->load());
    gainSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("out")->load()));

    for (int c = 0; c < 2; ++c)
    {
        held[c] = 0.f;  holdCount[c] = 0;
        toneState[c] = 0.f;  dcX[c] = 0.f;  dcY[c] = 0.f;
    }
    ringPhase = 0.0;
}

void PinkyLiteAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh = juce::jmin (2, buffer.getNumChannels());
    const int n = buffer.getNumSamples();
    if (numCh == 0 || n == 0)
        return;

    for (int c = numCh; c < buffer.getNumChannels(); ++c)
        buffer.clear (c, 0, n);

    if ((int) lfoBuf.size() < n)   lfoBuf.resize ((size_t) n);
    if ((int) phaseBuf.size() < n) phaseBuf.resize ((size_t) n);

    const float twoPi = juce::MathConstants<float>::twoPi;

    // --- темп и позиция от FL Studio ---
    double bpm = 120.0, ppq = internalPpq;
    bool playing = false;

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm())  bpm = *b;
            playing = pos->getIsPlaying();
            if (playing)
                if (auto p = pos->getPpqPosition())
                    ppq = *p;
        }

    const double ppqPerSample = bpm / 60.0 / sr;

    // --- параметры ---
    auto get = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };

    const int   rateIdx  = (int) get ("rate");
    const int   shape    = (int) get ("shape");
    const float volDepth = get ("volDepth");
    const int   filtType = (int) get ("filtType");
    const float baseCut  = get ("cutoff");
    const float reso     = get ("reso");
    const float cutMod   = get ("cutMod");
    const int   distType = (int) get ("distType");
    const float drive    = get ("drive");
    const float bias     = get ("bias");
    const float tone     = get ("tone");
    const float drvMod   = get ("drvMod");
    const float crush    = get ("crush");
    const float ringAmt  = get ("ringAmt");
    const float ringFreq = get ("ringFreq");
    const float panDepth = get ("pan");
    const int   osMode   = juce::jlimit (0, 2, (int) get ("os"));
    const bool  safe     = get ("safe") > 0.5f;

    mixSm.setTargetValue (get ("mix"));
    gainSm.setTargetValue (juce::Decibels::decibelsToGain (get ("out")));

    // смена режима oversampling -> сброс фильтров и новая задержка
    if (osMode != appliedMode)
    {
        appliedMode = osMode;
        os2.reset();
        os4.reset();
        setLatencySamples (osLatency[osMode]);
    }
    const int delay = osLatency[osMode];

    float* data[2] = { buffer.getWritePointer (0), numCh > 1 ? buffer.getWritePointer (1) : nullptr };

    // --- 1) LFO на каждый сэмпл ---
    const double cycle = beatsPerCycle[juce::jlimit (0, 5, rateIdx)];
    float lastPhase = 0.f;
    for (int i = 0; i < n; ++i)
    {
        const double q = ppq + i * ppqPerSample;
        double ph = std::fmod (q / cycle, 1.0);
        if (ph < 0.0) ph += 1.0;
        lastPhase = (float) ph;
        phaseBuf[(size_t) i] = lastPhase;
        lfoBuf[(size_t) i] = lfoShape (shape, lastPhase);
    }

    // --- 2) запоминаем сухой сигнал (с компенсацией задержки oversampling) ---
    const int mask = ringSize - 1;
    for (int ch = 0; ch < numCh; ++ch)
    {
        auto& r = dryRing[ch];
        for (int i = 0; i < n; ++i)
            r[(size_t) ((ringPos + i) & mask)] = data[ch][i];
    }

    // --- 3) фильтр, который качает LFO ---
    const bool filterOff = (filtType == 0 && baseCut >= 19990.f && cutMod < 0.001f)
                        || (filtType == 1 && baseCut <= 20.5f && cutMod < 0.001f);
    if (! filterOff)
    {
        filter.setType (filtType == 0 ? juce::dsp::StateVariableTPTFilterType::lowpass
                       : filtType == 1 ? juce::dsp::StateVariableTPTFilterType::highpass
                                       : juce::dsp::StateVariableTPTFilterType::bandpass);
        filter.setResonance (reso);

        for (int i = 0; i < n; ++i)
        {
            const float lfo = lfoBuf[(size_t) i];
            const float cutoff = juce::jlimit (20.0f, 20000.0f,
                                               baseCut * std::pow (2.0f, cutMod * (lfo - 0.5f) * 10.0f));
            filter.setCutoffFrequency (cutoff);
            for (int ch = 0; ch < numCh; ++ch)
                data[ch][i] = filter.processSample (ch, data[ch][i]);
        }
    }

    // --- 4) дисторшн (с oversampling) ---
    const bool distActive = drive > 0.001f || std::abs (bias) > 0.001f;

    auto shapeBlock = [&] (juce::dsp::AudioBlock<float> blk, int baseStart, int fac)
    {
        const int len = (int) blk.getNumSamples();
        for (int j = 0; j < len; ++j)
        {
            const float lfo  = lfoBuf[(size_t) (baseStart + j / fac)];
            const float dEff = drive * (1.0f - drvMod * (1.0f - lfo));
            const float kk   = 1.0f + dEff * 20.0f;

            for (size_t ch = 0; ch < blk.getNumChannels(); ++ch)
            {
                float* p = blk.getChannelPointer (ch);
                p[j] = shapeSample (distType, (p[j] + bias * 0.2f) * kk);
            }
        }
    };

    {
        juce::dsp::AudioBlock<float> full (buffer.getArrayOfWritePointers(), (size_t) numCh, (size_t) n);

        if (osMode == 0)
        {
            if (distActive)
                shapeBlock (full, 0, 1);
        }
        else
        {
            auto& os = (osMode == 1) ? os2 : os4;
            const int factor = 1 << osMode;

            for (int start = 0; start < n; start += maxBlock)
            {
                const int len = juce::jmin (maxBlock, n - start);
                auto sub = full.getSubBlock ((size_t) start, (size_t) len);
                auto up = os.processSamplesUp (sub);
                if (distActive)
                    shapeBlock (up, start, factor);
                os.processSamplesDown (sub);
            }
        }
    }

    // --- 5) убираем постоянку и применяем Tone ---
    if (distActive)
    {
        const bool needDc = std::abs (bias) > 0.001f || distType == 3 || distType == 5;
        const float r = 1.0f - twoPi * 8.0f / (float) sr;
        const bool toneOn = tone < 0.995f;
        const float a = std::exp (-twoPi * 500.0f * std::pow (40.0f, tone) / (float) sr);

        for (int ch = 0; ch < numCh; ++ch)
            for (int i = 0; i < n; ++i)
            {
                float x = data[ch][i];
                if (needDc)
                {
                    const float y = x - dcX[ch] + r * dcY[ch];
                    dcX[ch] = x;
                    dcY[ch] = y;
                    x = y;
                }
                if (toneOn)
                {
                    toneState[ch] = (1.0f - a) * x + a * toneState[ch];
                    x = toneState[ch];
                }
                data[ch][i] = x;
            }
    }

    // --- 6) Ring mod ---
    if (ringAmt > 0.001f)
    {
        const double inc = (double) twoPi * ringFreq / sr;
        for (int i = 0; i < n; ++i)
        {
            const float s = (float) std::sin (ringPhase);
            ringPhase += inc;
            if (ringPhase > (double) twoPi) ringPhase -= (double) twoPi;

            for (int ch = 0; ch < numCh; ++ch)
            {
                const float x = data[ch][i];
                data[ch][i] = x * (1.0f - ringAmt) + x * s * ringAmt;
            }
        }
    }

    // --- 7) Crush: битность + sample-hold ---
    if (crush > 0.001f)
    {
        const float bits = 16.0f - crush * 13.0f;                 // 16 -> 3 бита
        const float step = std::pow (2.0f, bits - 1.0f);
        const int   hold = 1 + (int) (crush * crush * 24.0f);

        for (int i = 0; i < n; ++i)
            for (int ch = 0; ch < numCh; ++ch)
            {
                if (++holdCount[ch] >= hold)
                {
                    holdCount[ch] = 0;
                    held[ch] = std::round (data[ch][i] * step) / step;
                }
                data[ch][i] = held[ch];
            }
    }

    // --- 8) Volume shaper + автопан ---
    const bool panOn = numCh == 2 && panDepth > 0.001f;
    for (int i = 0; i < n; ++i)
    {
        const float vg = 1.0f - volDepth * (1.0f - lfoBuf[(size_t) i]);
        float gl = vg, gr = vg;

        if (panOn)
        {
            const float p = 0.5f + 0.5f * panDepth * std::sin (twoPi * phaseBuf[(size_t) i]);
            const float ang = p * juce::MathConstants<float>::halfPi;
            gl *= juce::MathConstants<float>::sqrt2 * std::cos (ang);
            gr *= juce::MathConstants<float>::sqrt2 * std::sin (ang);
        }

        data[0][i] *= gl;
        if (data[1] != nullptr)
            data[1][i] *= gr;
    }

    // --- 9) Mix, Gain, Safe limiter ---
    for (int i = 0; i < n; ++i)
    {
        const float mix = mixSm.getNextValue();
        const float g   = gainSm.getNextValue();
        const size_t ri = (size_t) ((ringPos + i - delay + ringSize) & mask);

        for (int ch = 0; ch < numCh; ++ch)
        {
            float y = (dryRing[ch][ri] * (1.0f - mix) + data[ch][i] * mix) * g;
            if (! std::isfinite (y)) y = 0.0f;
            if (safe) y = softLimit (y);
            data[ch][i] = y;
        }
    }

    ringPos = (ringPos + n) & mask;
    internalPpq = ppq + n * ppqPerSample;
    uiPhase.store (lastPhase);
}

juce::AudioProcessorEditor* PinkyLiteAudioProcessor::createEditor()
{
    return new PinkyLiteAudioProcessorEditor (*this);
}

void PinkyLiteAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void PinkyLiteAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PinkyLiteAudioProcessor();
}
