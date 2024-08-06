#include "BiquadFilter.hpp"

BiquadFilter::BiquadFilter(int sampleRate) : sampleRate(sampleRate) {}

BiquadFilter::BiquadFilter(struct BiquadFilterCoefficients coeffs,
                           int sampleRate)
    : sampleRate(sampleRate), coefficients(coeffs) {}

BiquadFilter::BiquadFilter(enum BiquadFilterType type,
                           struct BiquadFilterParams params, int sampleRate)
    : sampleRate(sampleRate) {
    this->setParameters(type, params);
}

void BiquadFilter::setParameters(enum BiquadFilterType type,
                                 struct BiquadFilterParams params) {
    this->type = type;
    this->parameters = params;

    this->updateParameters();
}

void BiquadFilter::setSampleRate(int sampleRate) {
    this->sampleRate = sampleRate;

    this->updateParameters();
}

void BiquadFilter::updateParameters() {
    if (this->type == UNKOWN) return;

    if (this->parameters.Q == 0) return;

    const float A = std::pow(10, this->parameters.gain / 40),
                sqrtA = std::sqrt(A);
    const float om = M_PI * 2 * this->parameters.f / this->sampleRate;
    const float a = std::sin(om) / (2 * this->parameters.Q);
    const float cs = std::cos(om), sn = std::sin(om);
    switch (this->type) {
        case ALLPASS:
            this->coefficients = {.b0 = 1 - a,
                                  .b1 = -2 * cs,
                                  .b2 = 1 + a,
                                  .a0 = 1 + a,
                                  .a1 = -2 * cs,
                                  .a2 = 1 - a};
            break;
        case LOWPASS:
            this->coefficients = {.b0 = (1 - cs) / 2,
                                  .b1 = 1 - cs,
                                  .b2 = (1 - cs) / 2,
                                  .a0 = 1 + a,
                                  .a1 = -2 * cs,
                                  .a2 = 1 - a};
            break;
        case HIGHPASS:
            this->coefficients = {.b0 = (1 + cs) / 2,
                                  .b1 = -1 - cs,
                                  .b2 = (1 + cs) / 2,
                                  .a0 = 1 + a,
                                  .a1 = -2 * cs,
                                  .a2 = 1 - a};
            break;
        case BANDPASS:
            this->coefficients = {.b0 = a,
                                  .b1 = 0,
                                  .b2 = -a,
                                  .a0 = 1 + a,
                                  .a1 = -2 * cs,
                                  .a2 = 1 - a};
            break;
        case NOTCH:
            this->coefficients = {.b0 = 1,
                                  .b1 = -2 * cs,
                                  .b2 = 1,
                                  .a0 = 1 + a,
                                  .a1 = -2 * cs,
                                  .a2 = 1 - a};
            break;
        case PEAK:
            this->coefficients = {.b0 = 1 + a * A,
                                  .b1 = -2 * cs,
                                  .b2 = 1 - a * A,
                                  .a0 = 1 + a / A,
                                  .a1 = -2 * cs,
                                  .a2 = 1 - a / A};
            break;
        case LOWSHELF:
            this->coefficients = {
                .b0 = A * ((A + 1) - (A - 1) * cs + 2 * sqrtA * a),
                .b1 = 2 * A * ((A - 1) - (A + 1) * cs),
                .b2 = A * ((A + 1) - (A - 1) * cs - 2 * sqrtA * a),
                .a0 = (A + 1) + (A - 1) * cs + 2 * sqrtA * a,
                .a1 = -2 * ((A - 1) + (A + 1) * cs),
                .a2 = (A + 1) + (A - 1) * cs - 2 * sqrtA * a};
            break;
        case HIGHSHELF:
            this->coefficients = {
                .b0 = A * ((A + 1) + (A - 1) * cs + 2 * sqrtA * a),
                .b1 = -2 * A * ((A - 1) + (A + 1) * cs),
                .b2 = A * ((A + 1) + (A - 1) * cs - 2 * sqrtA * a),
                .a0 = (A + 1) - (A - 1) * cs + 2 * sqrtA * a,
                .a1 = 2 * ((A - 1) - (A + 1) * cs),
                .a2 = (A + 1) - (A - 1) * cs - 2 * sqrtA * a};
            break;
        default:
            break;
    }
}

void BiquadFilter::setParameters(struct BiquadFilterCoefficients coeffs) {
    this->coefficients = coeffs;
    this->type = UNKOWN;
}

void BiquadFilter::processBlock(float* buffer, int size,
                                struct SOState& state) const {
    for (int i = 0; i < size; i++) {
        float yn = coefficients.b0 * buffer[i] + coefficients.b1 * state.x1 +
                   coefficients.b2 * state.x2 - coefficients.a1 * state.y1 -
                   coefficients.a2 * state.y2;
        yn /= coefficients.a0;
        state.x2 = state.x1;
        state.x1 = buffer[i];
        state.y2 = state.y1;
        state.y1 = yn;
        buffer[i] = yn;
    }
}