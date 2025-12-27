//
//  GrPartials.cpp
//  GrainLib
//
//  Created by Roald Christesen on 24.01.24.
//

#include "DSP/Partials.hpp"
#include "Type/Type.hpp"
#include "DSP/Freq.hpp"
#include "DSP/LUT1.hpp"
#include "Bezier/BezierValueCurve.hpp"
#include "Signal/Audio.hpp"


namespace Grain {

    [[nodiscard]] float PartialsRange::t(int32_t index) const {
        if (index < 0 || m_base_freq <= 0.0f || m_p_min < 0.0f || m_p_max <= 0.0f || m_p_max <= m_p_min) {
            return -1.0f; // Invalid
        }
        const float bin_freq = m_base_freq * (index + 1);
        const float bin_pitch = Audio::pitchFromFreq(bin_freq);
        return (bin_pitch - m_p_min) / (m_p_max - m_p_min);
    }


    [[nodiscard]] float PartialsRange::f(int32_t index) const {
        auto t = this->t(index);
        return t < 0.0f ? 0.0f : DSP::hanningWindow(t);
    }


    Partials::Partials(int32_t resolution) noexcept : Object() {
        m_resolution = resolution;
        m_ma = static_cast<float*>(std::malloc(memSize()));
        m_ph = m_ma ? &m_ma[m_resolution] : nullptr;
        m_use_extern_mem = false;
        clear();
    }


    Partials::Partials(int32_t resolution, float* mem) noexcept : Object() {
        m_resolution = resolution;
        m_ma = mem;
        m_ph = m_ma ? &m_ma[m_resolution] : nullptr;
        m_use_extern_mem = true;
        clear();
    }


    Partials::Partials(const Partials* other) noexcept : Object() {
        if (other) {
            m_resolution = other->resolution();
            m_ma = static_cast<float*>(std::malloc(memSize()));
            m_ph = m_ma ? &m_ma[m_resolution] : nullptr;
            m_use_extern_mem = false;
            m_dc = other->m_dc;
            if (m_ma && other->m_ma) {
                memcpy(m_ma, other->m_ma, memSize());
            }
            m_use_extern_mem = false;
        }
    }


    Partials::~Partials() noexcept {
        if (m_ma && !m_use_extern_mem) {
            std::free(m_ma);
        }
    }

    float Partials::mag(int32_t bin_index) const noexcept {
        if (isBin(bin_index)) {
            return m_ma[bin_index];
        }
        return 0.0f;
    }


    float Partials::phase(int32_t bin_index) const noexcept {
        if (isBin(bin_index)) {
            return m_ph[bin_index];
        }
        return 0.0f;
    }


    float Partials::magInterpolated(float bin_index) const noexcept {
        if (bin_index <= 0.0f) {
            return mag(0);
        }

        if (bin_index >= static_cast<float>(m_resolution)) {
            return mag(m_resolution);
        }

        auto i0 = static_cast<int32_t>(bin_index);
        auto i1 = i0 + 1;
        float f1 = bin_index - static_cast<float>(i0);
        float f0 = 1.0f - f1;

        return mag(i0) * f0 + mag(i1) * f1;
    }


    float Partials::magLerp(float t) const noexcept {
        return magInterpolated(t * static_cast<float>(m_resolution));
    }


    void Partials::clear() noexcept {
        if (hasData()) {
            memset(m_ma, 0, memSize());
        }
    }


    float Partials::magMax() const noexcept {
        float max = 0.0f;
        for (int32_t i = 0; i < m_resolution; i++) {
            if (m_ma[i] > max) {
                max = m_ma[i];
            }
        }
        return max;
    }


    void Partials::setMag(int32_t bin_index, float mag) noexcept {
        if (isBin(bin_index)) {
            m_ma[bin_index] = mag;
        }
    }


    void Partials::addMag(int32_t bin_index, float mag) noexcept {
        if (isBin(bin_index)) {
            m_ma[bin_index] += mag;
        }
    }


    void Partials::scaleMag(int32_t bin_index, float mag) noexcept {
        if (isBin(bin_index)) {
            m_ma[bin_index] *= mag;
        }
    }


    void Partials::setMagNyquist(float mag) noexcept {
        m_ma[m_resolution - 1] = mag;
        m_ph[m_resolution - 1] = 0.0f;
    }


    void Partials::setPhase(int32_t bin_index, float phase) noexcept {
        if (isBin(bin_index)) {
            m_ph[bin_index] = phase;
        }
    }


    bool Partials::set(const Partials* other) noexcept {
        if (other && other->m_resolution == m_resolution && hasData() && other->hasData()) {
            memcpy(m_ma, other->m_ma, memSize());
            return true;
        }
        else {
            return false;
        }
    }


    void Partials::setByFormantData(int32_t n, const float* formant_data, int32_t sample_rate) noexcept {
        float f = static_cast<float>(m_resolution - 1) / (static_cast<float>(sample_rate) / 2.0f);
        float base_freq = static_cast<float>(sample_rate) / static_cast<float>(m_resolution * 2);

        clear();

        for (int32_t i = 0; i < n; i++) {
            float freq = formant_data[i * 3 + 0];
            float db  = formant_data[i * 3 + 1];
            float mag  = Audio::dbToLinear(db);
            float bw_cent = formant_data[i * 3 + 2] * 4;

            PartialsRange partials_range;
            rangeAtFreq(base_freq, freq, bw_cent, partials_range);

            if (partials_range.m_window_sum > 0.0001f) {
                for (int32_t j = partials_range.m_min_bin_index; j <= partials_range.m_max_bin_index; j++) {
                    addMag(j, mag * partials_range.f(j));
                }
            }
        }
    }


    bool Partials::accumulateProduct(const Partials* a, const Partials* b) noexcept {
        if (!a || !b) return false;
        if (a->m_resolution != m_resolution || b->m_resolution != m_resolution) {
            return false; // resolution mismatch
        }

        // Handle DC separately
        {
            // Convert polar to complex
            float ar = a->m_dc; // if m_dc is magnitude of bin0 (phase = 0)
            float br = b->m_dc;
            // DC is real-only, so product is just multiply
            m_dc += ar * br;
        }

        // Accumulate over bins
        for (int i = 0; i < m_resolution; ++i) {
            // Convert polar -> cartesian
            float ma = a->m_ma[i];
            float ph_a = a->m_ph[i];
            float ar = ma * std::cos(ph_a);
            float ai = ma * std::sin(ph_a);

            float mb = b->m_ma[i];
            float ph_b = b->m_ph[i];
            float br = mb * std::cos(ph_b);
            float bi = mb * std::sin(ph_b);

            // Complex multiply
            float pr = ar * br - ai * bi;
            float pi = ar * bi + ai * br;

            // Existing accumulator value in cartesian
            float mc = m_ma[i];
            float ph_c = m_ph[i];
            float cr = mc * std::cos(ph_c);
            float ci = mc * std::sin(ph_c);

            // Accumulate
            cr += pr;
            ci += pi;

            // Back to polar
            m_ma[i] = std::sqrt(cr * cr + ci * ci);
            m_ph[i] = std::atan2(ci, cr);
        }
        return true;
    }


    bool Partials::interpolate(int32_t first_index, int32_t last_index, int16_t mode) noexcept {
        /* TODO: !!!! Implement!
        if (first_index < 0 || first_index > m_resolution || last_index < 0 || last_index > m_resolution) {
            return false;
        }

        if (first_index > last_index) {
            std::swap(first_index, last_index);
        }

        float firstValue = valueAtIndex(first_index, mode);
        float lastValue = valueAtIndex(last_index, mode);

        for (int32_t i = first_index + 1; i < last_index; i++) {
            float f = static_cast<float>(i - first_index) / (last_index - first_index);
            float v = firstValue + f * (lastValue - firstValue);
            setValueAtIndex(i, v, mode);
        }
        */
        return true;
    }


    void Partials::setByFreqRange(
            const FreqBands& freq_bands,
            int32_t sample_rate,
            float amount,
            float* lut_mem,
            int32_t lut_length)
            noexcept {
        constexpr int32_t kMaxStackLUTLength = 1024;
        float lut_stack_mem[kMaxStackLUTLength];
        double root_freq = static_cast<double>(sample_rate) / m_resolution / 2;
        double high_freq = root_freq * (m_resolution - 1);

        double t_left = Freq::freqToPos(freq_bands.leftFreq(), root_freq, high_freq, 0, 1);
        double t_center = Freq::freqToPos(freq_bands.centerFreq(), root_freq, high_freq, 0, 1);
        double t_right = Freq::freqToPos(freq_bands.rightFreq(), root_freq, high_freq, 0, 1);

        if (!lut_mem) {
            lut_mem = lut_stack_mem;
            lut_length = kMaxStackLUTLength;
        }

        LUT1 lut(lut_length, lut_mem);

        lut.setEaseValuesInRange(Math::EaseMode::OutQuint, t_left, t_center, 0.0f, 1.0f);
        lut.setEaseValuesInRange(Math::EaseMode::InQuint, t_center, t_right, 1.0f, 0.0f);

        setMag(0, 0);

        for (int32_t i = 1; i < m_resolution; i++) {
            double freq = root_freq * i;
            double pos = Freq::freqToPos(freq, root_freq, high_freq, 0, 1);
            float value = lut.lookup(static_cast<float>(pos));
            setMag(i, value * amount);
        }
    }


    void Partials::setByBezierValueCurve(
            BezierValueCurve* bezier_value_curve,
            int32_t sample_rate,
            float lo_hz,
            float hi_hz,
            float shift_hz
    ) noexcept
    {
        if (bezier_value_curve) {
            float lo_pitch = Audio::pitchFromFreq(lo_hz);
            float hi_pitch = Audio::pitchFromFreq(hi_hz);
            std::cout << "lo_hz: " << lo_hz << ", hi_hz: " << hi_hz;
            std::cout << ", lo_pitch: " << lo_pitch;
            std::cout << ", hi_pitch: " << hi_pitch << std::endl;

            for (int32_t bin_index = 0; bin_index < m_resolution; bin_index++) {
                auto bin_hz =
                        static_cast<float>(bin_index + 1) /
                        static_cast<float>(m_resolution) *
                        static_cast<float>(sample_rate) * 0.5f;
                auto bin_pitch = Audio::pitchFromFreq(bin_hz);
                auto t = Math::remapf(lo_pitch, hi_pitch, 0.0f, 1.0f, bin_pitch);


                t = Math::remapf(lo_hz, hi_hz, 0.0f, 1.0f, bin_hz);
                float value = bezier_value_curve->lookup(t, 2048);
                setMag(bin_index, value);
            }
        }
    }


    void Partials::setByEQ21(const EQ21& eq) noexcept {
        // TODO: Implement
    }


    Partials* Partials::doubleResolution() noexcept {
        // TODO: !!!! Check!
        auto expanded = new (std::nothrow) Partials(m_resolution * 2);
        if (!expanded) {
            return nullptr;
        }

        expanded->setDC(m_dc);
        auto o = expanded->mutMagPtr();

        int32_t last_bin_index = expanded->resolution();
        for (int32_t bin_index = 1; bin_index <= last_bin_index; bin_index++) {
            auto t = Math::remapf(static_cast<float>(bin_index), static_cast<float>(last_bin_index), 0.0f, 1.0f, static_cast<float>(bin_index));
            o[bin_index] = magLerp(t);
        }

        return expanded;
    }


    void Partials::rangeAtFreq(
            float base_freq, float freq, float cent_range,
            PartialsRange& out_range
    ) const noexcept
    {
        out_range.clear();
        out_range.m_base_freq = base_freq;

        float semitone_range = cent_range / 100.0f;
        float pitch = Audio::pitchFromFreq(freq);
        float p_min = pitch - semitone_range;
        float p_max = pitch + semitone_range;
        out_range.m_p_min = p_min;
        out_range.m_p_max = p_max;

        for (int32_t bin_index = 0; bin_index < m_resolution; bin_index++) {
            float bin_freq = base_freq * (bin_index + 1);
            float bin_pitch = Audio::pitchFromFreq(bin_freq);

            if (bin_pitch >= p_min && bin_pitch <= p_max) {
                auto t = (bin_pitch - p_min) / (p_max - p_min);

                if (out_range.m_min_bin_index < 0) {
                    out_range.m_min_bin_index = bin_index;
                    out_range.m_min_t = t;
                }
                out_range.m_max_bin_index = bin_index;
                out_range.m_max_t = t;
                out_range.count_++;
                out_range.m_window_sum += DSP::hanningWindow(t);
            }
            else if (bin_pitch > p_max) {
                return;
            }
        }
    }


} // End of namespace Grain
