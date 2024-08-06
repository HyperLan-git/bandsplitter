#include "SelectorComponent.hpp"

SelectorComponent::SelectorComponent(juce::AudioParameterChoice* param)
    : listener(param, this->comboBox),
      label(param->getName(128), param->getName(128)) {
    this->addAndMakeVisible(this->comboBox);
    this->addAndMakeVisible(this->label);

    this->label.setJustificationType(juce::Justification::centred);

    this->comboBox.setLookAndFeel(&lf);
    this->comboBox.setColour(juce::ComboBox::buttonColourId,
                             juce::Colours::grey.brighter());
    this->comboBox.setColour(juce::ComboBox::backgroundColourId,
                             juce::Colours::grey.contrasting());
    this->comboBox.setJustificationType(juce::Justification::centred);

    this->comboBox.addItemList(param->getAllValueStrings(), 1);
    this->comboBox.setSelectedId(param->getIndex() + 1);

    setSize(200, 40);
}

SelectorComponent::~SelectorComponent() {
    this->comboBox.setLookAndFeel(nullptr);
}

void SelectorComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
}

void SelectorComponent::resized() {
    this->label.setBounds(0, 0, 200, 20);
    this->comboBox.setBounds(0, 20, 200, 20);
}

SelectorListener::SelectorListener(juce::AudioParameterChoice* param,
                                   juce::ComboBox& comboBox)
    : comboBox(comboBox), param(param) {
    param->addListener(this);
    comboBox.addListener(this);
}

void SelectorListener::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) {
    int id = comboBoxThatHasChanged->getSelectedId();
    if (id == 0) return;
    param->setValueNotifyingHost(param->convertTo0to1(id - 1));
}

void SelectorListener::parameterValueChanged(int parameterIndex,
                                             float newValue) {
    (void)parameterIndex;
    int id = param->getIndex() + 1;
    if (this->comboBox.getSelectedId() != id) this->comboBox.setSelectedId(id);
}

void SelectorListener::parameterGestureChanged(int parameterIndex,
                                               bool gestureIsStarting) {
    (void)parameterIndex;
    (void)gestureIsStarting;
}

SelectorListener::~SelectorListener() {
    param->removeListener(this);
    comboBox.removeListener(this);
}
