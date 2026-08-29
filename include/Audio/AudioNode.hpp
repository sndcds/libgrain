#pragma once

#include <cstdint>

namespace Grain {

    /**
     * Base DSP node:
     * - runs in audio thread
     * - no allocations allowed inside process()
     */
    class AudioNode {
    public:
        virtual ~AudioNode() = default;

        virtual void prepare(uint32_t sampleRate, uint32_t maxFrames) {}

        virtual void process(float** outputs,
                             const float** inputs,
                             uint32_t frames,
                             uint32_t channels) = 0;
    };

}