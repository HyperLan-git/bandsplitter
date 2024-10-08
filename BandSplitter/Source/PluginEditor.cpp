#include "PluginEditor.hpp"

// TODO replace those shitty knobs omfg

BandSplitterAudioProcessorEditor::BandSplitterAudioProcessorEditor(
    BandSplitterAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      addBand("ADD"),
      removeBand("REMOVE"),
      bands("bands", "Bands : " + std::to_string(*p.getBandParam())),
      listener(p.getBandParam(), bands) {
    juce::AudioParameterInt* bandParam = p.getBandParam();
    int b = *bandParam;
    for (int i = 0; i < MAX_BANDS - 1; i++) {
        this->splits[i].emplace(p.getFreqParam(i));
        this->addAndMakeVisible(*this->splits[i]);
        if (b < i + 2) this->splits[i]->setVisible(false);
    }

    this->addAndMakeVisible(addBand);
    this->addAndMakeVisible(removeBand);
    this->addAndMakeVisible(bands);

    bands.setJustificationType(juce::Justification::centred);

    addBand.onClick = [=]() {
        int val = *bandParam + 1;
        bandParam->setValueNotifyingHost(bandParam->convertTo0to1(val));
        if (val < MAX_BANDS + 1) splits[val - 2]->setVisible(true);
    };
    removeBand.onClick = [=]() {
        int val = *bandParam - 1;
        bandParam->setValueNotifyingHost(bandParam->convertTo0to1(val));
        if (val > 1) splits[val - 1]->setVisible(false);
    };

    setSize(800, 500);
}

BandSplitterAudioProcessorEditor::~BandSplitterAudioProcessorEditor() {}

void BandSplitterAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
}

void BandSplitterAudioProcessorEditor::resized() {
    addBand.setBounds(0, 0, 100, 50);
    removeBand.setBounds(100, 0, 100, 50);
    bands.setBounds(200, 0, 100, 50);
    for (int i = 0; i < MAX_BANDS - 1; i++) {
        this->splits[i]->setBounds((i % 8) * 100, 100 + (i / 8) * 100, 100,
                                   100);
    }
}

BandListener::BandListener(juce::AudioParameterInt* param, juce::Label& label)
    : param(param), label(label) {
    param->addListener(this);
}

BandListener::~BandListener() { param->removeListener(this); }

void BandListener::parameterValueChanged(int parameterIndex, float newValue) {
    label.setText("Bands : " + std::to_string(*param),
                  juce::NotificationType::sendNotificationAsync);
}
void BandListener::parameterGestureChanged(int parameterIndex,
                                           bool gestureIsStarting) {
    if (gestureIsStarting)
        label.setText("Bands : " + std::to_string(*param) + "~",
                      juce::NotificationType::sendNotificationAsync);
    else
        label.setText("Bands : " + std::to_string(*param),
                      juce::NotificationType::sendNotificationAsync);
}