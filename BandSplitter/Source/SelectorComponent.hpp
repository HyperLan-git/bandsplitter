#pragma once

#include "JuceHeader.h"
#include "Looknfeel.hpp"

class SelectorListener : public juce::ComboBox::Listener,
                         public juce::AudioProcessorParameter::Listener {
   public:
    SelectorListener(juce::AudioParameterChoice* param,
                     juce::ComboBox& comboBox);

    ~SelectorListener();

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void parameterValueChanged(int parameterIndex, float newValue) override;

    void parameterGestureChanged(int parameterIndex,
                                 bool gestureIsStarting) override;

   private:
    juce::ComboBox& comboBox;
    juce::AudioParameterChoice* param;
};

class SelectorComponent : public juce::Component {
   public:
    SelectorComponent(juce::AudioParameterChoice* param);
    ~SelectorComponent() override;

    void paint(juce::Graphics& g) override;

    void resized() override;

   private:
    juce::ComboBox comboBox;
    juce::Label label;

    Looknfeel lf;

    SelectorListener listener;
};
