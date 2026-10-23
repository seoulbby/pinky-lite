#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>

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

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> uiPhase { 0.0f }; // для анимации в интерфейсе

private:
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::SmoothedValue<float> mixSm, gainSm;

    float held[2] { 0.f, 0.f };
    int holdCount[2] { 0, 0 };

    double sr = 44100.0;
    double internalPpq = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PinkyLiteAudioProcessor)
};
