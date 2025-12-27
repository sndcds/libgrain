//
//  WeightedSamples.cpp
//
//  Created by Roald Christesen on 19.03.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "DSP/WeightedSamples.hpp"
#include "Type/Type.hpp"
#include "Math/Vec2.hpp"
#include "Bezier/Bezier.hpp"


namespace Grain {

    WeightedSamples::WeightedSamples(int32_t resolution) noexcept {
        resolution = std::clamp<int32_t>(resolution, MIN_RESOLUTION, MAX_RESOLUTION);

        m_resolution = 0;
        m_max_resolution = 0;
        m_samples = nullptr;
        m_weights = nullptr;

        auto err = setResolution(resolution);
        if (err != ErrorCode::None) {
            m_last_err = err;
        }

        m_use_extern_mem = false;
    }


    WeightedSamples::WeightedSamples(int32_t resolution, float *mem) noexcept {
        m_resolution = resolution;
        m_max_resolution = resolution;
        m_samples = mem;
        m_weights = &mem[resolution];
        m_use_extern_mem = true;
    }


    WeightedSamples::~WeightedSamples() noexcept {
        if (!m_use_extern_mem) {
            std::free(m_samples);
        }
    }


    ErrorCode WeightedSamples::setResolution(int32_t resolution) noexcept {
        if (resolution == m_resolution) {
            // Nothing changed, simply return
            return ErrorCode::None;
        }

        if (resolution < 1) {
            return Error::specific(ERR_RESOLUTION_NOT_ALLOWED);
        }

        if (m_use_extern_mem && resolution > m_max_resolution) {
            // External memory can not grow
            return Error::specific(ERR_RESOLUTION_NOT_ALLOWED);
        }

        if (resolution <= m_max_resolution) {
            // Requested resolution is lesser than current resolution
            m_resolution = resolution;
            return ErrorCode::None;
        }

        // Memory must grow
        auto err = ErrorCode::None;

        float* new_samples = nullptr;
        size_t new_size = sizeof(float) * 2 * resolution;

        if (!m_samples) {
            new_samples = (float*)std::malloc(new_size);
        }
        else {
            new_samples = (float*)std::realloc(m_samples, new_size);
        }

        if (new_samples) {
            m_samples = new_samples;
            m_weights = &new_samples[resolution];
            m_resolution = resolution;
            m_max_resolution = resolution;
        }
        else {
            err = ErrorCode::MemCantAllocate;
        }

        return err;
    }


    void WeightedSamples::clear() noexcept {
        Type::clearArray<float>(m_samples, m_resolution * 2);
    }


    void WeightedSamples::setSample(int32_t index, float value) noexcept {
        if (m_samples && index >= 0 && index < m_resolution) {
            m_samples[index] = value;
        }
    }


    float WeightedSamples::sampleAtIndex(int32_t index) const noexcept {
        return m_samples && index >= 0 && index < m_resolution ? m_samples[index] : 0;
    }


    void WeightedSamples::setWeight(int32_t index, float weight) noexcept {
        if (m_weights && index >= 0 && index < m_resolution) {
            m_weights[index] = weight;
        }
    }


    void WeightedSamples::addSample(float value, int32_t index, int32_t resolution) noexcept {
        if (m_weights && m_samples && index >= 0 && index < resolution && resolution > 0) {

            float t = static_cast<float>(index) / resolution;
            float real_index = t * m_resolution;
            int32_t index1 = std::floor(real_index);
            int32_t index2 = index1 + 1;

            float f2 = real_index - index1;
            float f1 = 1.0f - f2;

            if (index1 >= 0 && index1 < m_resolution) {
                m_samples[index1] += f1 * value;
                m_weights[index1] += f1;
            }

            if (index2 >= 0 && index2 < m_resolution) {
                m_samples[index2] += f2 * value;
                m_weights[index2] += f2;
            }
        }
    }


    void WeightedSamples::addWeightedSample(int32_t index, float value, float weight) noexcept {
        if (m_samples && m_weights && index >= 0 && index < m_resolution) {
            m_samples[index] += value;
            m_weights[index] += weight;
        }
    }


    float WeightedSamples::weightAtIndex(int32_t index) const noexcept {
        return m_weights && index >= 0 && index < m_resolution ? m_weights[index] : 0;
    }


    void WeightedSamples::minMaxY(float *out_min_y, float *out_max_y) const noexcept {
        float min = std::numeric_limits<float>::max();
        float max = std::numeric_limits<float>::lowest();

        if (m_samples) {
            float *s = m_samples;
            for (int32_t i = 0; i < m_resolution; i++) {
                min = std::min(min, *s);
                max = std::max(max, *s);
                s++;
            }
        }

        if (out_min_y) {
            *out_min_y = min;
        }

        if (out_max_y) {
            *out_max_y = max;
        }
    }


    float WeightedSamples::lookup(float t) const noexcept {
        float pos = t * m_resolution;
        int32_t index0 = std::floor(pos);
        int32_t index1 = index0 + 1;
        float f1 = pos - index0;
        float f0 = 1.0f - f1;

        if (index0 < 0) {
            return m_samples[0];
        }

        if (index0 >= m_resolution) {
            return m_samples[m_resolution - 1];
        }

        if (index0 >= 0 && index0 < m_resolution) {
            if (index1 > m_resolution - 1) {
                return m_samples[m_resolution - 1];
            }
            return f0 * m_samples[index0] + f1 * m_samples[index1];
        }

        return 0;
    }

    void WeightedSamples::addBezier(const Bezier& bezier, int32_t bezier_resolution) noexcept {
        if (!m_samples && bezier_resolution < 1) {
            return;
        }

        for (int32_t i = 0; i <= bezier_resolution; i++) {
            Vec2d sample = bezier.posOnCurve(static_cast<float>(i) / bezier_resolution);

            float t = sample.x_ * m_resolution;
            int32_t index1 = std::floor(t);
            int32_t index2 = index1 + 1;
            float f2 = t - index1;
            float f1 = 1.0f - f2;

            if (index1 >= 0 && index1 < m_resolution) {
                m_samples[index1] += f1 * sample.y_;
                m_weights[index1] += f1;
            }

            if (index2 >= 0 && index2 < m_resolution) {
                m_samples[index2] += f2 * sample.y_;
                m_weights[index2] += f2;
            }
        }
    }


    void WeightedSamples::finish() noexcept {
        if (!m_samples) {
            return;
        }

        // Normalize
        for (int32_t i = 0; i < m_resolution; i++) {
            m_samples[i] = m_weights[i] > 0.0f ? m_samples[i] /= m_weights[i] : std::numeric_limits<float>::max();
        }

        // Fill missing values
        int32_t n = 0;
        int32_t prev_index = -1;

        for (int32_t i = 0; i < m_resolution; i++) {
            if (m_samples[i] == std::numeric_limits<float>::max()) {    // Missing value.
                if (n == 0) {
                    prev_index = i - 1;
                }
                n++;
            }
            else if (n > 0) {
                double value = m_samples[prev_index];
                double step = (m_samples[i] - value) / (n + 2);
                for (int32_t j = 0; j < n; j++) {
                    value += step;
                    m_samples[prev_index + 1 + j] = value;
                }

                n = 0;
                prev_index = -1;
            }
        }

        if (prev_index >= 0) {
            for (int32_t i = prev_index + 1; i < m_resolution; i++) {
                m_samples[i] = m_samples[prev_index];
            }
        }
    }


} // End of namespace Grain
