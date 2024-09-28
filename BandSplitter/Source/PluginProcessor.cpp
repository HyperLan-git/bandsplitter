#include "PluginProcessor.hpp"

juce::AudioProcessor::BusesProperties
BandSplitterAudioProcessor::createProperties() {
    juce::AudioProcessor::BusesProperties result;
    result = result.withInput("Input", juce::AudioChannelSet::stereo(), true);
    for (int i = 0; i < MAX_BANDS; i++) {
        result = result.withOutput("Band " + std::to_string(i + 1),
                                   juce::AudioChannelSet::stereo(), true);
    }
    return result;
}

BandSplitterAudioProcessor::BandSplitterAudioProcessor()
    : AudioProcessor(createProperties()),
      bands(
          new juce::AudioParameterInt({"bands", 1}, "Bands", 2, MAX_BANDS, 3)),
      bandParams({nullptr}),
      type(new juce::AudioParameterChoice({"type", 1}, "Filter type",
                                          juce::StringArray{"Linkwitz-Riley 4"},
                                          0)) {
    this->addParameter(this->bands);
    this->addParameter(this->type);
    for (int i = 0; i < MAX_BANDS - 1; i++) {
        this->bandParams[i] = new juce::AudioParameterFloat(
            {"split freq " + std::to_string(i + 1), 1},
            "Split frequency " + std::to_string(i + 1), 20, 20000,
            100 + std::round(std::pow((float)i / MAX_BANDS, 2) * 200) * 100);
        this->addParameter(this->bandParams[i]);
    }
    this->filters[0].setParameters(LOWPASS,
                                   {.f = 20, .Q = .70710678118f, .gain = 0});
    this->filters[MAX_BANDS - 1].setParameters(
        HIGHPASS, {.f = 20, .Q = .70710678118f, .gain = 0});
}

BandSplitterAudioProcessor::~BandSplitterAudioProcessor() {}

const juce::String BandSplitterAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool BandSplitterAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool BandSplitterAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool BandSplitterAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double BandSplitterAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int BandSplitterAudioProcessor::getNumPrograms() { return 1; }

int BandSplitterAudioProcessor::getCurrentProgram() { return 0; }

void BandSplitterAudioProcessor::setCurrentProgram(int index) { (void)index; }

const juce::String BandSplitterAudioProcessor::getProgramName(int index) {
    (void)index;
    return {};
}

void BandSplitterAudioProcessor::changeProgramName(
    int index, const juce::String& newName) {
    (void)index;
    (void)newName;
}

void BandSplitterAudioProcessor::prepareToPlay(double sampleRate,
                                               int samplesPerBlock) {
    (void)sampleRate;
    (void)samplesPerBlock;
}

void BandSplitterAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BandSplitterAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    for (const auto& bus : layouts.getBuses(false)) {
        if (bus != juce::AudioChannelSet::mono() &&
            bus != juce::AudioChannelSet::stereo())
            return false;
    }

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() < layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

#define BUF(i) buffer.getWritePointer(i)

#define GET_STATE_BLOCK(ptr, BLK_SIZE, blk, filter, loops) \
    ((ptr) + ((blk) * (BLK_SIZE) + (filter) * (2 * (loops) + 2)))

void BandSplitterAudioProcessor::processMono(juce::AudioBuffer<float>& buffer) {
    const int inputs = getTotalNumInputChannels();
    const int outputs = getTotalNumOutputChannels();

    const int samples = buffer.getNumSamples();
    const double rate = getSampleRate();

    int n = *bands;
    if (n > outputs) n = outputs;
    if (lastBands != n) {
        lastBands = n;
        buffer.clear();
        for (int i = 0; i < n - 1; i++) {
            float f = *this->bandParams[i];
            filters[i].setParameters(LOWPASS,
                                     {.f = f, .Q = .70710678118f, .gain = 0});
            filters[i + MAX_BANDS - 1].setParameters(
                HIGHPASS, {.f = f, .Q = .70710678118f, .gain = 0});
        }
        return;
    }

    // Copy buffer data
    for (int i = 0; i < n - 1; i++) {
        std::memcpy(BUF(i + 1), BUF(i), samples * sizeof(float));
    }

    const int t = *type;

    for (int i = 0; i < n - 1; i++) {
        float *data = BUF(i), *next = BUF(i + 1);

        BiquadFilter &lp = filters[i], &hp = filters[i + MAX_BANDS - 1];

        // Update filter
        const float f = *this->bandParams[i];
        if (lp.getParameters().f != f) {
            lp.setParameters(LOWPASS, {.f = f, .Q = .70710678118f, .gain = 0});
            hp.setParameters(HIGHPASS, {.f = f, .Q = .70710678118f, .gain = 0});
        }

        // Run filter
        for (int j = 0; j <= i; j++) {
            lp.processBlockMul(BUF(j), samples,
                               GET_STATE_BLOCK(lp_states, STATE_BLK, j, i, 2),
                               2);
        }
        for (int j = n - 1; j > i; j--) {
            hp.processBlockMul(
                BUF(j - 1), samples,
                GET_STATE_BLOCK(hp_states, STATE_BLK, j - 1, i, 2), 2);
        }
    }
    if (!std::isfinite(BUF(0)[0])) {
        std::memset(lp_states, 0, sizeof(lp_states));
        std::memset(hp_states, 0, sizeof(hp_states));
    }
}

void BandSplitterAudioProcessor::processStereo(
    juce::AudioBuffer<float>& buffer) {
    const int inputs = getTotalNumInputChannels();
    const int outputs = getTotalNumOutputChannels();

    const int samples = buffer.getNumSamples();
    const double rate = getSampleRate();

    const size_t bufSize = samples * sizeof(float);

    int n = *bands;
    if (n * 2 > outputs) n = outputs / 2;
    if (lastBands != n) {
        lastBands = n;
        buffer.clear();
        for (int i = 0; i < n - 1; i++) {
            float f = *this->bandParams[i];
            filters[i].setParameters(LOWPASS,
                                     {.f = f, .Q = .70710678118f, .gain = 0});
            filters[i + MAX_BANDS - 1].setParameters(
                HIGHPASS, {.f = f, .Q = .70710678118f, .gain = 0});
        }
        return;
    }

    // Copy buffer data
    for (int i = 0; i < n - 1; i++) {
        std::memcpy(BUF(i * 2 + 2), BUF(i * 2), bufSize);
        std::memcpy(BUF(i * 2 + 3), BUF(i * 2 + 1), bufSize);
    }

    for (int i = 0; i < n - 1; i++) {
        float *l = BUF(i * 2), *r = BUF(i * 2 + 1);
        float *l2 = BUF(i * 2 + 2), *r2 = BUF(i * 2 + 3);

        BiquadFilter &lp = filters[i], &hp = filters[i + MAX_BANDS - 1];

        // Update filter
        const float f = *this->bandParams[i];
        if (lp.getParameters().f != f) {
            lp.setParameters(LOWPASS, {.f = f, .Q = .70710678118f, .gain = 0});
            hp.setParameters(HIGHPASS, {.f = f, .Q = .70710678118f, .gain = 0});
        }

        // Run filters
        for (int j = 0; j <= i; j++) {
            lp.processBlockMul(
                BUF(j * 2), samples,
                GET_STATE_BLOCK(lp_states, STATE_BLK, j * 2, i, 2), 2);
            lp.processBlockMul(
                BUF(j * 2 + 1), samples,
                GET_STATE_BLOCK(lp_states, STATE_BLK, j * 2 + 1, i, 2), 2);
        }
        for (int j = n - 1; j > i; j--) {
            hp.processBlockMul(
                BUF(j * 2), samples,
                GET_STATE_BLOCK(hp_states, STATE_BLK, (j - 1) * 2, i, 2), 2);
            hp.processBlockMul(
                BUF(j * 2 + 1), samples,
                GET_STATE_BLOCK(hp_states, STATE_BLK, (j - 1) * 2 + 1, i, 2),
                2);
        }
    }

    if (!std::isfinite(BUF(0)[0])) {
        std::memset(lp_states, 0, sizeof(lp_states));
        std::memset(hp_states, 0, sizeof(hp_states));
    }
}

void BandSplitterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages) {
    (void)midiMessages;

    juce::ScopedNoDenormals noDenormals;
    const int inputs = getTotalNumInputChannels();
    const int outputs = getTotalNumOutputChannels();

    if (inputs == 0) return;
    if (outputs == 0) return;
    if (outputs <= inputs) return;

    if (inputs == 1) {
        processMono(buffer);
    } else {
        processStereo(buffer);
    }
}

bool BandSplitterAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* BandSplitterAudioProcessor::createEditor() {
    return new BandSplitterAudioProcessorEditor(*this);
}

void BandSplitterAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData) {
    juce::MemoryOutputStream stream(destData, true);
    stream.writeInt(MAX_BANDS);
    stream.writeFloat(GET_PARAM_NORMALIZED(bands));
    stream.writeFloat(GET_PARAM_NORMALIZED(type));
    for (int i = 0; i < MAX_BANDS - 1; i++) {
        stream.writeFloat(GET_PARAM_NORMALIZED(bandParams[i]));
    }
}

void BandSplitterAudioProcessor::setStateInformation(const void* data,
                                                     int sizeInBytes) {
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes),
                                   false);
    int n = stream.readInt();
    if (n > MAX_BANDS) n = MAX_BANDS;

    bands->setValueNotifyingHost(stream.readFloat());
    type->setValueNotifyingHost(stream.readFloat());
    for (int i = 0; i < n - 1; i++) {
        this->bandParams[i]->setValueNotifyingHost(stream.readFloat());
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new BandSplitterAudioProcessor();
}
