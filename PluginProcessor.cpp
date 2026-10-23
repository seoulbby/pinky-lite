#include "PluginProcessor.h"
#include "PluginEditor.h"

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

juce::AudioProcessorValueTreeState::ParameterLayout PinkyLiteAudioProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;

    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "rate", 1 },  "Rate",  rateNames(),  2));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "shape", 1 }, "Shape", shapeNames(), 0));

    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "volDepth", 1 }, "Volume Depth",
                                                  NormalisableRange<float> (0.f, 1.f), 0.7f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "cutoff", 1 }, "Cutoff",
                                                  NormalisableRange<float> (20.f, 20000.f, 0.f, 0.3f), 20000.f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "reso", 1 }, "Resonance",
                                                  NormalisableRange<float> (0.5f, 10.f, 0.f, 0.5f), 0.8f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "cutMod", 1 }, "Filter Mod",
                                                  NormalisableRange<float> (0.f, 1.f), 0.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "drive", 1 }, "Drive",
                                                  NormalisableRange<float> (0.f, 1.f), 0.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "crush", 1 }, "Crush",
                                                  NormalisableRange<float> (0.f, 1.f), 0.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "pan", 1 }, "Auto Pan",
                                                  NormalisableRange<float> (0.f, 1.f), 0.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "mix", 1 }, "Mix",
                                                  NormalisableRange<float> (0.f, 1.f), 1.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "out", 1 }, "Output",
                                                  NormalisableRange<float> (-24.f, 12.f), 0.0f));
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

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    filter.prepare (spec);
    filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.reset();

    mixSm.reset (sampleRate, 0.02);
    gainSm.reset (sampleRate, 0.02);
    mixSm.setCurrentAndTargetValue (*apvts.getRawParameterValue ("mix"));
    gainSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("out")->load()));

    held[0] = held[1] = 0.f;
    holdCount[0] = holdCount[1] = 0;
}

void PinkyLiteAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh = juce::jmin (2, buffer.getNumChannels());
    const int numSamples = buffer.getNumSamples();

    for (int c = numCh; c < buffer.getNumChannels(); ++c)
        buffer.clear (c, 0, numSamples);

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
    const float baseCut  = get ("cutoff");
    const float reso     = get ("reso");
    const float cutMod   = get ("cutMod");
    const float drive    = get ("drive");
    const float crush    = get ("crush");
    const float panDepth = get ("pan");

    mixSm.setTargetValue (get ("mix"));
    gainSm.setTargetValue (juce::Decibels::decibelsToGain (get ("out")));

    const double cycle = beatsPerCycle[juce::jlimit (0, 5, rateIdx)];
    const float  k = 1.0f + drive * 12.0f;
    const float  kNorm = 1.0f / std::tanh (k);
    const float  bits = 16.0f - crush * 13.0f;                 // 16 -> 3 бита
    const float  step = std::pow (2.0f, bits - 1.0f);
    const int    hold = 1 + (int) (crush * crush * 24.0f);     // понижение частоты дискретизации

    filter.setResonance (reso);

    float* data[2] = { buffer.getWritePointer (0), numCh > 1 ? buffer.getWritePointer (1) : nullptr };
    float lastPhase = 0.f;

    for (int i = 0; i < numSamples; ++i)
    {
        const double q = ppq + i * ppqPerSample;
        double ph = std::fmod (q / cycle, 1.0);
        if (ph < 0.0) ph += 1.0;
        lastPhase = (float) ph;

        const float lfo = lfoShape (shape, lastPhase);
        const float volGain = 1.0f - volDepth * (1.0f - lfo);
        const float cutoff = juce::jlimit (20.0f, 20000.0f,
                                           baseCut * std::pow (2.0f, cutMod * (lfo - 0.5f) * 10.0f));
        filter.setCutoffFrequency (cutoff);

        const float mix = mixSm.getNextValue();
        const float gain = gainSm.getNextValue();

        float dry[2] { 0.f, 0.f }, wet[2] { 0.f, 0.f };

        for (int ch = 0; ch < numCh; ++ch)
        {
            float x = data[ch][i];
            dry[ch] = x;

            // 1) фильтр (с модуляцией от LFO)
            x = filter.processSample (ch, x);

            // 2) drive
            if (drive > 0.001f)
                x = std::tanh (x * k) * kNorm;

            // 3) crush: битность + sample-hold
            if (crush > 0.001f)
            {
                if (++holdCount[ch] >= hold)
                {
                    holdCount[ch] = 0;
                    held[ch] = std::round (x * step) / step;
                }
                x = held[ch];
            }

            // 4) volume shaper
            x *= volGain;
            wet[ch] = x;
        }

        // 5) автопан (только стерео)
        if (numCh == 2 && panDepth > 0.001f)
        {
            const float p = 0.5f + 0.5f * panDepth * std::sin (juce::MathConstants<float>::twoPi * lastPhase);
            const float a = p * juce::MathConstants<float>::halfPi;
            wet[0] *= juce::MathConstants<float>::sqrt2 * std::cos (a);
            wet[1] *= juce::MathConstants<float>::sqrt2 * std::sin (a);
        }

        for (int ch = 0; ch < numCh; ++ch)
            data[ch][i] = (dry[ch] * (1.0f - mix) + wet[ch] * mix) * gain;
    }

    internalPpq = ppq + numSamples * ppqPerSample;
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
