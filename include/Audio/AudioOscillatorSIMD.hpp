#pragma once

#include "AudioNode.hpp"
#include "AudioSIMD.hpp"
#include <cmath>

namespace Grain {

    class AudioOscillatorSIMD : public AudioNode {
    public:
        void setFrequency(float f) {
            freq_ = f;
        }

        void prepare(uint32_t sampleRate, uint32_t) override {
            sr_ = sampleRate;
            phaseInc_ = 2.0f * 3.14159265359f * freq_ / sr_;
        }

        void process(float** outputs,
                     const float**,
                     uint32_t frames,
                     uint32_t channels) override
        {
            uint32_t i = 0;

            // SIMD step = 4 samples at a time
            for (; i + 4 <= frames; i += 4) {

                float p0 = phase_;
                float p1 = p0 + phaseInc_;
                float p2 = p1 + phaseInc_;
                float p3 = p2 + phaseInc_;

                float s[4] = {
                    std::sin(p0),
                    std::sin(p1),
                    std::sin(p2),
                    std::sin(p3)
                };

                phase_ = p3 + phaseInc_;

                for (uint32_t ch = 0; ch < channels; ch++) {
                    outputs[ch][i + 0] = s[0];
                    outputs[ch][i + 1] = s[1];
                    outputs[ch][i + 2] = s[2];
                    outputs[ch][i + 3] = s[3];
                }
            }

            // tail
            for (; i < frames; i++) {
                float s = std::sin(phase_);
                phase_ += phaseInc_;

                for (uint32_t ch = 0; ch < channels; ch++) {
                    outputs[ch][i] = s;
                }
            }
        }

    private:
        float freq_ = 440.0f;
        float phase_ = 0.0f;
        float phaseInc_ = 0.0f;
        uint32_t sr_ = 44100;
    };

}