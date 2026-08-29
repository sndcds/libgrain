#pragma once

#include "AudioNode.hpp"
#include "AudioRingBuffer.hpp"

namespace Grain {

    class AudioGraph {
    public:
        void setOutputNode(AudioNode* node) {
            output_ = node;
        }

        void setRingBuffer(AudioRingBuffer* buffer) {
            buffer_ = buffer;
        }

        void process(uint32_t frames, uint32_t channels) {
            if (!output_ || !buffer_) return;

            float* channelsPtr[32];

            for (uint32_t ch = 0; ch < channels; ch++) {
                channelsPtr[ch] = temp_[ch];
            }

            output_->process(channelsPtr, nullptr, frames, channels);

            buffer_->writePlanar(channelsPtr, frames, channels);
        }

    private:
        AudioNode* output_ = nullptr;
        AudioRingBuffer* buffer_ = nullptr;

        float temp_[32][2048]{};
    };

}