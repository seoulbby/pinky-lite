#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <vector>

// Pinky Lite - made by seoulbby
class PinkyLiteAudioProcessor : public juce::AudioProcessor
{
public:
    PinkyLiteAudioProcessor();
    ~PinkyLiteAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Pinky Lite"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Форма LFO: 0..1 (1 = полная громкость)
    static float lfoShape (int shape, float phase);

    static const juce::StringArray& rateNames();
    static const juce::StringArray& shapeNames();
    static const juce::StringArray& filterNames();
    static const juce::StringArray& distNames();
    static const juce::StringArray& osNames();
    static const juce::StringArray& themeNames();

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> uiPhase { 0.0f }; // для анимации в интерфейсе

private:
    static float shapeSample (int type, float u);
    static float softLimit (float y);

    juce::dsp::StateVariableTPTFilter<float> filter;

    juce::dsp::Oversampling<float> os2 { 2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true };
    juce::dsp::Oversampling<float> os4 { 2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true };

    juce::SmoothedValue<float> mixSm, gainSm;

    std::vector<float> lfoBuf, phaseBuf;

    static constexpr int ringSize = 16384; // кольцевой буфер для сухого сигнала (компенсация задержки)
    std::vector<float> dryRing[2];
    int ringPos = 0;

    float held[2] { 0.f, 0.f };
    int holdCount[2] { 0, 0 };
    float toneState[2] { 0.f, 0.f };
    float dcX[2] { 0.f, 0.f };
    float dcY[2] { 0.f, 0.f };

    double ringPhase = 0.0;
    double sr = 44100.0;
    double internalPpq = 0.0;
    int maxBlock = 512;

    int osLatency[3] { 0, 0, 0 };
    int appliedMode = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PinkyLiteAudioProcessor)
};
