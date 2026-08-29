#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>

namespace Grain {

class AudioRingBuffer {
public:
    bool init(size_t frames, size_t channels) {
        channels_ = channels;
        capacity_ = frames;

        buffer_ = new float[capacity_ * channels_];
        if (!buffer_) return false;

        std::memset(buffer_, 0, capacity_ * channels_ * sizeof(float));

        writeIndex_.store(0);
        readIndex_.store(0);
        return true;
    }

    ~AudioRingBuffer() {
        delete[] buffer_;
    }

    // ----------------------------
    // PLANAR WRITE (DSP thread)
    // ----------------------------
    bool writePlanar(const float* const* input,
                      size_t frames,
                      size_t channels)
    {
        if (channels != channels_) return false;

        size_t w = writeIndex_.load(std::memory_order_relaxed);

        for (size_t i = 0; i < frames; i++) {
            size_t base = ((w + i) % capacity_) * channels_;

            for (size_t ch = 0; ch < channels; ch++) {
                buffer_[base + ch] = input[ch][i];
            }
        }

        writeIndex_.store(w + frames, std::memory_order_release);
        return true;
    }

    // ----------------------------
    // INTERLEAVED WRITE (optional)
    // ----------------------------
    bool writeInterleaved(const float* input,
                          size_t frames,
                          size_t channels)
    {
        if (channels != channels_) return false;

        size_t w = writeIndex_.load(std::memory_order_relaxed);

        for (size_t i = 0; i < frames; i++) {
            size_t base = ((w + i) % capacity_) * channels_;

            std::memcpy(
                &buffer_[base],
                &input[i * channels],
                channels * sizeof(float)
            );
        }

        writeIndex_.store(w + frames, std::memory_order_release);
        return true;
    }

    // ----------------------------
    // READ (audio thread)
    // ----------------------------
    bool read(float* output,
              size_t frames,
              size_t channels)
    {
        if (channels != channels_) return false;

        size_t r = readIndex_.load(std::memory_order_relaxed);
        size_t w = writeIndex_.load(std::memory_order_acquire);

        size_t available = w - r;
        if (available < frames)
            frames = available;

        for (size_t i = 0; i < frames; i++) {
            size_t base = ((r + i) % capacity_) * channels_;

            std::memcpy(
                &output[i * channels],
                &buffer_[base],
                channels * sizeof(float)
            );
        }

        readIndex_.store(r + frames, std::memory_order_release);
        return true;
    }

private:
    float* buffer_ = nullptr;

    size_t capacity_ = 0;
    size_t channels_ = 0;

    std::atomic<size_t> writeIndex_{0};
    std::atomic<size_t> readIndex_{0};
};

}