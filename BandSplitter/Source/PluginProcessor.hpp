#pragma once

constexpr int MAX_BANDS = 16;  // TODO , MAX_ORDER = 4;

#include "JuceHeader.h"

#include "PluginEditor.hpp"
#include "BiquadFilter.hpp"

#define GET_PARAM_NORMALIZED(param) (param->convertTo0to1(*param))
#define SET_PARAM_NORMALIZED(param, value) \
    param->setValueNotifyingHost(param->convertTo0to1(value))

class BandSplitterAudioProcessor : public juce::AudioProcessor {
   public:
    BandSplitterAudioProcessor();
    ~BandSplitterAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    inline juce::AudioParameterInt* getBandParam() { return bands; }
    inline juce::AudioParameterFloat* getFreqParam(int split) {
        // assert(split < MAX_BANDS - 1);
        return bandParams[split];
    }

   private:
    juce::AudioProcessor::BusesProperties createProperties();

    void processMono(juce::AudioBuffer<float>& buffer);
    void processStereo(juce::AudioBuffer<float>& buffer);

    int lastBands = 0;
    // We have (bands - 1) splits
    juce::AudioParameterInt* bands;
    // juce::AudioParameterInt* order;
    std::array<juce::AudioParameterFloat*, MAX_BANDS - 1> bandParams;

    BiquadFilter filters[MAX_BANDS - 1] = {};

    struct SOState states[4 * MAX_BANDS - 4] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BandSplitterAudioProcessor)
};
