#pragma once

#include <array>

#include "JuceHeader.h"

#include "PluginProcessor.hpp"
#include "KnobComponent.hpp"

class BandSplitterAudioProcessor;

class BandListener : public juce::AudioProcessorParameter::Listener {
   public:
    BandListener(juce::AudioParameterInt* param, juce::Label& label);

    ~BandListener() override;

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex,
                                 bool gestureIsStarting) override;

   private:
    juce::AudioParameterInt* param;
    juce::Label& label;
};

class BandSplitterAudioProcessorEditor : public juce::AudioProcessorEditor {
   public:
    BandSplitterAudioProcessorEditor(BandSplitterAudioProcessor&);
    ~BandSplitterAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

   private:
    BandSplitterAudioProcessor& audioProcessor;

    juce::TextButton addBand;
    juce::TextButton removeBand;
    juce::Label bands;

    BandListener listener;

    std::array<std::optional<KnobComponent>, MAX_BANDS - 1> splits = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        BandSplitterAudioProcessorEditor)
};
