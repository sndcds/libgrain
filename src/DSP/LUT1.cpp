//
//  LUT1.cpp
//
//  Created by Roald Christesen on 07.11.2013
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "DSP/LUT1.hpp"
#include "Type/Type.hpp"
#include "Math/Math.hpp"
#include "DSP/WeightedSamples.hpp"
/* TODO: !!!!!
#include <Bezier.hpp>
#include <LevelCurve.hpp>
*/

namespace Grain {

    LUT1::LUT1(int32_t resolution) noexcept {
        _init(resolution, resolution, nullptr);
    }


    LUT1::LUT1(int32_t resolution, int32_t max_sesolution) noexcept {
        _init(resolution, max_sesolution, nullptr);
    }


    LUT1::LUT1(int32_t resolution, float* mem) noexcept {
        _init(resolution, resolution, mem);
    }


    LUT1::~LUT1() noexcept {

        if (!m_use_extern_mem) {
            std::free(m_samples);
        }
    }


    void LUT1::_init(int32_t resolution, int32_t max_sesolution, float* mem) noexcept {
        resolution = std::clamp<int32_t>(resolution, kMinResolution, kMaxResolution);
        max_sesolution = std::clamp<int32_t>(max_sesolution, kMinResolution, kMaxResolution);

        if (max_sesolution < resolution) {
            max_sesolution = (resolution / kDefaultResolution + 1) * kDefaultResolution;
        }

        m_resolution = resolution;
        m_max_resolution = max_sesolution;
        m_last_index = resolution - 1;

        if (mem) {
            m_samples = mem;
            m_use_extern_mem = true;
        }
        else {
            m_samples = (float*)std::malloc(max_sesolution * sizeof(float));
        }

        clear();
    }


    ErrorCode LUT1::setResolution(int32_t resolution) noexcept {
        resolution = std::clamp<int32_t>(resolution, kMinResolution, kMaxResolution);

        if (resolution <= m_max_resolution) {
            m_resolution = resolution;
            m_last_index = resolution - 1;
        }
        else {
            if (m_use_extern_mem) {
                return ErrorCode::MemExternalMemCantGrow;
            }

            float* new_samples = (float*)std::realloc(m_samples, resolution * sizeof(float));
            if (!new_samples) {
                return ErrorCode::MemCantAllocate;
            }

            m_samples = new_samples;
            m_resolution = resolution;
            m_max_resolution = resolution;
            m_last_index = resolution - 1;
        }

        return ErrorCode::None;
    }


    const float* LUT1::samplePtrAtIndex(int32_t index) const noexcept {
        return !m_samples || index < 0 || index >= m_resolution ? nullptr : &m_samples[index];
    }


    float* LUT1::mutSamplePtrAtIndex(int32_t index) const noexcept {
        return !m_samples || index < 0 || index >= m_resolution ? nullptr : &m_samples[index];
    }


    void LUT1::clear() noexcept {
        clear(0);
    }


    void LUT1::clear(float value) noexcept {
        Type::fillStridedArray<float>(m_samples, 0, 1, m_resolution, m_resolution, 0);
    }


    void LUT1::limit(float min, float max) noexcept {
        Type::clampData(m_samples, m_resolution, min, max);
    }


    void LUT1::scale(float scale) noexcept {
        if (m_samples) {
            for (int32_t i = 0; i < m_resolution; i++) {
                m_samples[i] *= scale;
            }
        }
    }


    void LUT1::gamma(float e) noexcept {
        if (m_samples) {
            for (int32_t i = 0; i < m_resolution; i++) {
                m_samples[i] = std::pow(m_samples[i], e);
            }
        }
    }


    void LUT1::add(const LUT1* lut) noexcept {
        if (m_samples) {
            for (int32_t i = 0; i < m_resolution; i++) {
                m_samples[i] += lut->lookup(static_cast<float>(i) / m_last_index);
            }
        }
    }


    void LUT1::mul(const LUT1* lut) noexcept {

        if (m_samples) {
            for (int32_t i = 0; i < m_resolution; i++) {
                m_samples[i] *= lut->lookup(static_cast<float>(i) / m_last_index);
            }
        }
    }


    void LUT1::hanningWindow() noexcept {
        DSP::hanningWindowSymmetric(m_resolution, m_samples);
    }


    void LUT1::window(DSP::WindowType window_type, float alpha, float beta, bool unity_gain) noexcept {
        DSP::window(m_resolution, window_type, alpha, beta, unity_gain, m_samples);
    }


    void LUT1::normalize() noexcept {
        if (m_samples) {
            Type::normalizeArrayToUnitRange<float>(m_samples, m_resolution);
        }
    }


    void LUT1::alignZero() noexcept {
        if (m_samples) {
            float min = Type::minOfArray<float>(m_samples, m_resolution);
            for (int32_t i = 0; i < m_resolution; i++) {
                m_samples[i] -= min;
            }
        }
    }


    void LUT1::flip() noexcept {
        Type::flipArray(m_samples, m_resolution);
    }


    void LUT1::setValueAtIndex(int32_t index, float value) noexcept {
        if (m_samples && index >= 0 && index < m_resolution) {
            m_samples[index] = value;
        }
    }


    void LUT1::setValuesInIndexRange(int32_t start_index, int32_t end_index, float value) noexcept {
        if (m_samples) {
            start_index = std::clamp(start_index, 0, m_last_index);
            end_index = std::clamp(end_index, start_index, m_last_index);

            for (int32_t i = start_index; i <= end_index; i++) {
                m_samples[i] = value;
            }
        }
    }


    void LUT1::setValuesInRange(float value, float t_left, float t_right) noexcept {
        setValuesInIndexRange(tToIndex(t_left), tToIndex(t_right), value);
    }


    void LUT1::setEaseValuesInRange(Math::EaseMode ease_mode, float t_left, float t_right, float value_left, float value_right) noexcept {
        int32_t start_index = tToIndex(t_left);
        int32_t end_index = tToIndex(t_right);

        if (start_index >= end_index || start_index > m_last_index || end_index < 0) {
            return;
        }

        double t = 0.0;
        double t_step = 1.0 / (end_index - start_index);
        if (start_index < 0) {
            t = t_step * -start_index;
            start_index = 0;
        }
        if (end_index > m_last_index) {
            end_index = m_last_index;
        }
        for (int32_t lut_index = start_index; lut_index < end_index; lut_index++) {
            if (lut_index >= 0 && lut_index <= m_last_index) {
                float v = Math::ease(ease_mode, t);
                v = Math::remap(0.0, 1.0, value_left, value_right, v);
                setValueAtIndex(lut_index, v);
            }
            t += t_step;
        }
    }

/* TODO: !!!!!
    void LUT1::setByBezier(const Bezier& bezier, int32_t bezier_resolution) noexcept {

        if (!m_samples && m_resolution > 2) {
            return;
        }

        bezier_resolution = std::min(bezier_resolution, static_cast<int32_t>(kMaxbezierResolution));

        // Compute bezier samples
        Vec2d bezier_samples[bezier_resolution];
        bezier.buildVec2LUT(bezier_samples, bezier_resolution);

        for (int32_t i = 0; i < m_resolution; i++) {
            float x = static_cast<float>(i) / m_last_index;

            for (int32_t j = 0; j < bezier_resolution - 1; j++) {
                if (x >= bezier_samples[j].m_x && x <= bezier_samples[j + 1].m_x) {
                    float length = bezier_samples[j + 1].m_x - bezier_samples[j].m_x;
                    if (length < std::numeric_limits<float>::epsilon()) {
                        m_samples[i] = bezier_samples[j].m_y;
                    }
                    else {
                        float f0 = (x - bezier_samples[j].m_x) / length;
                        float f1 = 1.0f - f0;
                        m_samples[i] = f0 * bezier_samples[j + 1].m_y + f1 * bezier_samples[j].m_y;
                    }
                    break;
                }
            }
        }
    }
*/

/* TODO: !!!!!
    void LUT1::setByLevelCurve(const LevelCurve& level_curve) noexcept {

        for (int32_t i = 0; i < m_resolution; i++) {
            m_samples[i] = level_curve.yAtX(Math::remapnorm(0, m_resolution - 1, i));
        }
    }
*/

    bool LUT1::setByWeightedSamples(const WeightedSamples* weighted_samples) noexcept {
        if (!m_samples || !weighted_samples) {
            return false;
        }

        if (m_resolution != weighted_samples->resolution()) {
            return false;
        }

        Type::copy(mutSamplePtr(), weighted_samples->samplesPtr(), m_resolution);

        return true;
    }


    bool LUT1::setByGaussKernel(double sigma_sqr) noexcept {
        if (!m_samples) {
            return false;
        }

        double radius = 4.0; // TODO: radius, is 4.0 the correct value?
        double x = -radius;
        double xStep = radius * 2 / (m_resolution - 1);
        double sum = 0.0;

        for (int32_t lut_index = 0; lut_index < m_resolution; lut_index++) {
            double v = Math::gaussKernel(x, sigma_sqr);
            setValueAtIndex(lut_index, v);
            sum += v;
            x += xStep;
        }

        if (sum <= 0.0) {
            return false;
        }

        for (int32_t i = 0; i < m_resolution; i++) {
            m_samples[i] /= sum;
        }

        return m_resolution % 2;
    }



    ErrorCode LUT1::interpolateGaps(float* min, float* max) noexcept {
        // TODO: Other interpolation curves, not just linear.

        float min_value = std::numeric_limits<float>::max();
        float max_value = std::numeric_limits<float>::lowest();

        int32_t gap_start_index = -1;
        int32_t gap_end_index = 0;
        for (int32_t i = 0; i <= m_last_index; i++) {

            float value = valueAtIndex(i);

            if (value == std::numeric_limits<float>::max()) {
                if (gap_start_index < 0) {
                    gap_start_index = i;
                }
            }
            else if (gap_start_index >= 0) {
                gap_end_index = i - 1;

                double start_value = valueAtIndex(gap_start_index - 1);
                double end_value = valueAtIndex(gap_end_index + 1);
                double value_step = (end_value - start_value) / (gap_end_index - gap_start_index + 2);
                double curr_value = start_value + value_step;

                for (int32_t gi = gap_start_index; gi <= gap_end_index; gi++) {
                    setValueAtIndex(gi, static_cast<float>(curr_value));
                    curr_value += value_step;
                }

                gap_start_index = -1;
            }
            else {
                if (value < min_value) {
                    min_value = value;
                }
                if (value > max_value) {
                    max_value = value;
                }
            }
        }

        if (gap_start_index >= 0) {
            float value = valueAtIndex(gap_start_index - 1);
            for (int32_t gi = gap_start_index; gi <= m_last_index; gi++) {
                setValueAtIndex(gi, value);
            }
        }

        return ErrorCode::None;
    }


    void LUT1::vibrato(int32_t sample_rate, int32_t n, int32_t offset, float amount, float freq, float delay, float attack, float duration, float release) noexcept {
        if (amount < std::numeric_limits<float>::min() || n < 1 || sample_rate < 1) {
            return;
        }

        freq = std::clamp(freq, 0.0001f, static_cast<float>(sample_rate) / 2);
        delay = std::clamp(delay, 0.0f, std::numeric_limits<float>::max());
        attack = std::clamp(attack, 0.0f, std::numeric_limits<float>::max());
        duration = std::clamp(duration, 0.0f, std::numeric_limits<float>::max());
        release = std::clamp(release, 0.0f, std::numeric_limits<float>::max());

        n = std::clamp(n, 0, m_resolution);

        int32_t index0 = std::clamp(static_cast<int32_t>(delay * sample_rate), 0, std::numeric_limits<int32_t>::max());
        if (index0 >= n) {
            return;
        }

        int32_t index1 = std::clamp(static_cast<int32_t>((delay + attack) * sample_rate), index0, std::numeric_limits<int32_t>::max());
        int32_t index2 = std::clamp(static_cast<int32_t>((delay + attack + duration)) * sample_rate, index1, std::numeric_limits<int32_t>::max());
        int32_t index3 = std::clamp(static_cast<int32_t>((delay + attack + duration + release) * sample_rate), index2, std::numeric_limits<int32_t>::max());


        double t_step = std::numbers::pi * 2 / sample_rate * freq;
        double t = Type::wrappedValue<double>(static_cast<double>(offset + index0) * t_step, 0.0, std::numbers::pi * 2);

        // Attack.

        float *d = &m_samples[index0];
        double f_step = 1.0 / (index1 - index0);
        int32_t m = std::clamp(index1, 0, n);
        for (int32_t i = index0; i < m; i++) {

            double f = f_step * (i - index0);
            double s = std::sin(t) * amount * f;
            *d += s;
            if (*d <= 0.001f) {
                *d = 0.001f;
            }

            d++;
            t += t_step;
            if (t >= std::numbers::pi * 2) {
                t -= std::numbers::pi * 2;
            }
        }

        // Duration
        if (index1 >= n) {
            return;
        }

        m = std::clamp(index2, 0, n);
        for (int32_t i = index1; i < m; i++) {

            double s = std::sin(t) * amount;
            *d += s;
            if (*d <= 0.001f) {
                *d = 0.001f;
            }

            d++;
            t += t_step;
            if (t >= std::numbers::pi * 2) {
                t -= std::numbers::pi * 2;
            }
        }

        // Release
        if (index2 >= n) {
            return;
        }

        f_step = 1.0 / (index3 - index2);
        m = std::clamp(index3, 0, n);
        for (int32_t i = index2; i < m; i++) {

            double f = 1.0 - f_step * (i - index2);
            double s = std::sin(t) * amount * f;
            *d += s;
            if (*d <= 0.001f) {
                *d = 0.001f;
            }

            d++;
            t += t_step;
            if (t >= std::numbers::pi * 2) {
                t -= std::numbers::pi * 2;
            }
        }
    }


    int32_t LUT1::countAbove(float value) const noexcept {
        if (!m_samples) {
            return -1;
        }

        int32_t n = 0;
        for (int32_t i = 0; i < m_resolution; i++) {
            if (m_samples[i] > value) {
                n++;
            }
        }

        return n;
    }


    float LUT1::lookup(float t) const noexcept {
        if (!m_samples) {
            return 0.0f;
        }

        if (t <= 0) {
            return m_samples[0];
        }
        if (t >= 1) {
            return m_samples[m_last_index];
        }

        float real_index = tToIndex(t);
        int32_t index0 = static_cast<int32_t>(real_index);
        int32_t index1 = index0 + 1;
        if (index0 >= m_last_index) {
            return m_samples[m_last_index];
        }

        float f1 = real_index - index0;
        float f0 = 1.0f - f1;
        return f0 * m_samples[index0] + f1 * m_samples[index1];
    }


    float LUT1::valueAtRealIndex(float real_index) const noexcept {
        if (!m_samples) {
            return 0;
        }

        if (real_index < 0) {
            return m_samples[0];
        }

        int32_t index0 = static_cast<int32_t>(real_index);
        int32_t index1 = index0 + 1;
        if (index0 >= m_last_index) {
            return m_samples[m_last_index];
        }

        float f1 = real_index - index0;
        float f0 = 1.0f - f1;

        return f0 * m_samples[index0] + f1 * m_samples[index1];
    }


    float LUT1::firstValue() const noexcept {
        return valueAtIndex(0);
    }


    float LUT1::lastValue() const noexcept {
        return valueAtIndex(m_last_index);
    }


    float LUT1::valueAtIndex(int32_t index) const noexcept {
        if (m_samples) {
            if (index < 0) {
                return m_samples[0];
            }
            if (index > m_last_index) {
                return m_samples[m_last_index];
            }
            return m_samples[index];
        }

        return 0;
    }


    LUT1Stepper::LUT1Stepper(LUT1* lut, float duration, float step) noexcept {
        m_lut = lut;
        setDurationAndStep(duration, step);
    }


    void LUT1Stepper::start() noexcept {
        m_pos = 0.0;
        m_running = true;
    }


    bool LUT1Stepper::next(float* out_value) noexcept {
        return next(m_step, out_value);
    }


    bool LUT1Stepper::next(float step, float* out_value) noexcept {
        bool result = m_pos < m_duration;

        if (!m_running) {
            return false;
        }

        if (m_pos >= m_duration) {
            m_pos = m_duration;
            m_running = false;
        }

        if (out_value) {
            if (m_lut) {
                *out_value = m_min + m_lut->lookup(m_pos / m_duration) * (m_max - m_min);
            }
            else {
                *out_value = m_min;
            }
        }

        m_pos += step;

        return result;
    }


    void LUT1Stepper::setDuration(float duration) noexcept {
        m_duration = duration < 0.000001f ? 0.000001f : duration;
        m_pos = 0.0f;
    }


    void LUT1Stepper::setDurationAndStep(float duration, float step) noexcept {
        m_duration = duration < 0 ? 0 : duration;
        m_step = step < 0.000001f ? 0.000001f : step;
        m_pos = 0.0f;
    }


    void LUT1Stepper::setRange(float min, float max) noexcept {
        m_min = min;
        m_max = max;
    }


} // End of namespace Grain.
