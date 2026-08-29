#pragma once

#include "AudioNode.hpp"
#include <vector>

namespace Grain {

    class AudioMixerNode : public AudioNode {
    public:
        void addInput(AudioNode* node) {
            inputs_.push_back(node);
        }

        void process(float** outputs,
                     const float**,
                     uint32_t frames,
                     uint32_t channels) override
        {
            // clear output
            for (uint32_t ch = 0; ch < channels; ch++)
                for (uint32_t i = 0; i < frames; i++)
                    outputs[ch][i] = 0.0f;

            // sum inputs
            float** temp = outputs;

            for (auto* node : inputs_) {
                node->process(temp, nullptr, frames, channels);
            }
        }

    private:
        std::vector<AudioNode*> inputs_;
    };

}