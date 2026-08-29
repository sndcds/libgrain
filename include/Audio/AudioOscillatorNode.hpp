#pragma once

#include "AudioNode.hpp"
#include <cmath>

namespace Grain {

    class AudioOscillatorNode : public AudioNode {
    public:
        void setFrequency(float f) { freq_ = f; }

        void prepare(uint32_t sampleRate, uint32_t) override {
            sampleRate_ = sampleRate;
            phase_ = 0.0f;
            phaseInc_ = 2.0f * 3.14159265359f * freq_ / sampleRate_;
        }

        void process(float** outputs,
                     const float**,
                     uint32_t frames,
                     uint32_t channels) override
        {
            for (uint32_t i = 0; i < frames; i++) {
                float sample = std::sin(phase_);
                phase_ += phaseInc_;
                if (phase_ > 6.2831853f) phase_ -= 6.2831853f;

                for (uint32_t ch = 0; ch < channels; ch++) {
                    outputs[ch][i] = sample;
                }
            }
        }

    private:
        float freq_ = 440.0f;
        float phase_ = 0.0f;
        float phaseInc_ = 0.0f;
        uint32_t sampleRate_ = 44100;
    };

}