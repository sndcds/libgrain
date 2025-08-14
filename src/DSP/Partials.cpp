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

#include "Bezier/Bezier.hpp"
#include "Bezier/BezierValueCurve.hpp"
#include "DSP/FFT.hpp"
#include "Signal/Audio.hpp"


namespace Grain {


    Partials::Partials(int32_t resolution) noexcept : Object() {

        m_resolution = resolution;

        m_data_size = static_cast<size_t>(resolution) * sizeof(float) * 2;
        m_amplitude_data = (float*)std::malloc(m_data_size);
        m_phase_data = &m_amplitude_data[resolution];
        m_use_extern_mem = false;

        clear();
    }


    Partials::Partials(int32_t resolution, float* mem) noexcept : Object() {

        m_resolution = resolution;

        m_data_size = sizeof(float) * m_resolution * 2;
        m_amplitude_data = mem;
        m_phase_data = &m_amplitude_data[resolution];
        m_use_extern_mem = true;

        clear();
    }


    Partials::Partials(const Partials& partials) noexcept : Object() {

        m_resolution = partials.m_resolution;
        m_amplitude_data = (float*)std::malloc(sizeof(float) * m_resolution * 2);
        m_phase_data = &m_amplitude_data[m_resolution];

        for (int32_t i = 0; i < m_resolution; i++) {
            m_amplitude_data[i] = partials.m_amplitude_data[i];
            m_phase_data[i] = partials.m_phase_data[i];
        }

        m_use_extern_mem = false;
    }


    Partials::~Partials() noexcept {

        if (m_amplitude_data != nullptr && m_use_extern_mem == false) {
            std::free(m_amplitude_data);
        }
    }


    void Partials::partialAtIndex(int32_t index, float& out_amplitude, float& out_phase) const noexcept {

        if (isPartial(index)) {
            out_amplitude = m_amplitude_data[index];
            out_phase = m_phase_data[index];
        }
    }


    float Partials::amplitudeAtIndex(int32_t index) const noexcept {

        return isPartial(index) ? m_amplitude_data[index] : 0;
    }


    float Partials::amplitudeLerpAtIndex(float index) const noexcept {

        if (index <= 0) {
            return m_amplitude_data[0];
        }

        if (index >= (m_resolution - 1)) {
            return m_amplitude_data[m_resolution - 1];
        }

        int32_t i0 = static_cast<int32_t>(index);
        int32_t i1 = i0 + 1;
        float f1 = index - i0;
        float f0 = 1.0f - f1;

        return m_amplitude_data[i0] * f0 + m_amplitude_data[i1] * f1;
    }


    float Partials::amplitudeAtNormalizedIndex(float t) const noexcept {

        return amplitudeLerpAtIndex(t * (m_resolution - 1));
    }


    void Partials::scaleAmplitudeAtIndex(int32_t index, float scale) noexcept {

        if (isPartial(index)) {
            m_amplitude_data[index] *= scale;
        }
    }


    float Partials::phaseAtIndex(int32_t index) const noexcept {

        return isPartial(index) ? m_phase_data[index] : 0;
    }


    float Partials::valueAtIndex(int32_t index, int16_t mode) const noexcept {

        return isPartial(index) ? (mode == kModeAmplutude ? m_amplitude_data[index] : m_phase_data[index]) : 0;
    }


    void Partials::clear() noexcept {

        if (hasData()) {
            for (int32_t i = 0; i < m_resolution; i++) {
                m_amplitude_data[i] = 0;
                m_phase_data[i] = 0;
            }
        }
    }


    void Partials::clearAmplitudes() noexcept {

        setAmplitudes(0);
    }


    void Partials::setAmplitudes(float value) noexcept {

        if (hasData()) {
            for (int32_t i = 0; i < m_resolution; i++) {
                m_amplitude_data[i] = value;
            }
        }
    }


    void Partials::clearPhases() noexcept {

        setPhases(0);
    }


    void Partials::setPhases(float value) noexcept {

        if (hasData()) {
            for (int32_t i = 0; i < m_resolution; i++) {
                m_amplitude_data[i] = value;
            }
        }
    }


    void Partials::sawtooth() noexcept {

        if (hasData()) {
            m_amplitude_data[0] = 0;
            m_phase_data[0] = 0;
            for (int32_t i = 1; i < m_resolution; i++) {
                m_amplitude_data[i] = 1.0f / i;
                m_phase_data[i] = 0;
            }
        }
    }


    void Partials::square() noexcept {

        if (hasData()) {
            m_amplitude_data[0] = 1;
            m_phase_data[0] = 0;
            for (int32_t i = 1; i < m_resolution; i++) {
                m_amplitude_data[i] = i % 2 ? 1 / i : 0;
                m_phase_data[i] = 0;
            }
        }
    }


    void Partials::setValueAtIndex(int32_t index, float value, int16_t mode) noexcept {

        if (mode == kModeAmplutude) {
            setAmplitudeAtIndex(index, value);
        }
        else {
            setPhaseAtIndex(index, value);
        }
    }


    float Partials::maxAmplitude() const noexcept {

        float max = 0;

        for (int32_t i = 0; i < m_resolution; i++) {
            float a = m_amplitude_data[i];
            if (a > max) {
                max = a;
            }
        }

        return max;
    }


    void Partials::setPartialAtIndex(int32_t index, float amplitude, float phase) noexcept {

        if (isPartial(index)) {
            setAmplitudeAtIndex(index, amplitude);
            setPhaseAtIndex(index, phase);
        }
    }


    void Partials::setPartialsInRange(int32_t start_index, int32_t end_index, float amplitude, float phase) noexcept {

        start_index = std::clamp(start_index, 0, m_resolution - 1);
        end_index = std::clamp(end_index, start_index, m_resolution - 1);

        for (int32_t i = start_index; i <= end_index; i++) {
            setAmplitudeAtIndex(i, amplitude);
            setPhaseAtIndex(i, phase);
        }
    }


    void Partials::setAllPartials(float amplitude, float phase) noexcept {

        for (int32_t i = 0; i < m_resolution; i++) {
            setAmplitudeAtIndex(i, amplitude);
            setPhaseAtIndex(i, phase);
        }
    }


    void Partials::setAmplitudeAtIndex(int32_t index, float value) noexcept {

        if (isPartial(index)) {
            m_amplitude_data[index] = value;
        }
    }


    void Partials::setAmplitudesInRange(int32_t start_index, int32_t end_index, float value) noexcept {

        start_index = std::clamp(start_index, 0, m_resolution - 1);
        end_index = std::clamp(end_index, start_index, m_resolution - 1);

        for (int32_t i = start_index; i <= end_index; i++) {
            m_amplitude_data[i] = value;
        }
    }


    void Partials::setAllAmplitudes(float value) noexcept {

        for (int32_t i = 0; i < m_resolution; i++) {
            m_amplitude_data[i] = value;
        }
    }



    void Partials::setAllAmplitudes(const float* values) noexcept {

        if (values) {
            for (int32_t i = 0; i < m_resolution; i++) {
                m_amplitude_data[i] = values[i];
            }
        }
    }


    void Partials::addAmplitudeAtIndex(int32_t index, float value) noexcept {

        if (isPartial(index)) {
            m_amplitude_data[index] = m_amplitude_data[index] + value;
        }
    }


    void Partials::setPhaseAtIndex(int32_t index, float value) noexcept {

        if (isPartial(index)) {
            m_phase_data[index] = Type::wrappedBipolarPI<float>(value);
        }
    }


    void Partials::rotatePhaseAtIndex(int32_t index, float amount) noexcept {

        if (isPartial(index)) {
            m_phase_data[index] = Type::wrappedBipolarPI<float>(m_phase_data[index] + amount);
        }
    }


    void Partials::setPhasesInRange(int32_t start_index, int32_t end_index, float value) noexcept {

        start_index = std::clamp(start_index, 0, m_resolution - 1);
        end_index = std::clamp(end_index, start_index, m_resolution - 1);
        value = Type::wrappedBipolarPI<float>(value);

        for (int32_t i = start_index; i <= end_index; i++) {
            m_phase_data[i] = value;
        }
    }


    void Partials::setAllPhases(float value) noexcept {

        value = std::clamp(value, static_cast<float>(-std::numbers::pi), static_cast<float>(std::numbers::pi));

        for (int32_t i = 0; i < m_resolution ; i++) {
            m_phase_data[i] = value;
        }
    }


    bool Partials::set(const Partials* partials) noexcept {

        if (partials != nullptr) {
            int32_t n = std::min(m_resolution, partials->m_resolution);

            for (int32_t i = 0; i < n; i++) {
                m_amplitude_data[i] = partials->m_amplitude_data[i];
                m_phase_data[i] = partials->m_phase_data[i];
            }

            return true;
        }
        else {
            return false;
        }
    }


// TODO: Not used!
    inline void complex_mult(float ar, float ai, float br, float bi, float& out_r, float& out_i) {

        out_r =  (ar * br) + (ai * bi * -1.0);
        out_i =  ar * bi + ai * br;
    }


    bool Partials::mul(const Partials* partials) noexcept {

        if (partials != nullptr) {
            int32_t n = std::min(m_resolution, partials->m_resolution);
            for (int32_t i = 0; i < n; i++) {
                m_amplitude_data[i] *= partials->m_amplitude_data[i];
            }
            return true;
        }
        else {
            return false;
        }
    }


    bool Partials::complexMultiply(const Partials* partials) noexcept {

        if (partials != nullptr) {
            float* ar = m_amplitude_data;
            float* ai = m_phase_data;
            float* br = partials->m_amplitude_data;
            float* bi = partials->m_phase_data;

            int32_t n = std::min(m_resolution, partials->m_resolution);

            for (int32_t i = 0; i < n; i++) {
                float rr =  (ar[i] * br[i]) + (ai[i] * bi[i] * -1.0);
                float ri =  ar[i] * bi[i] + ai[i] * br[i];
                ar[i] = rr;
                ai[i] = ri;
            }

            return true;
        }
        else {
            return false;
        }
    }


    bool Partials::add(const Partials* partials) noexcept {

        if (partials != nullptr) {
            int32_t n = std::min(m_resolution, partials->m_resolution);
            for (int32_t i = 0; i < n; i++)
                m_amplitude_data[i] += partials->m_amplitude_data[i];

            return true;
        }
        else {
            return false;
        }
    }


    bool Partials::octaveUp() noexcept {

        // TODO: ... test!

        for (int32_t i = m_resolution - 1; i > 1; i--) {
            if (!(i % 2)) {
                int32_t j = i / 2;
                m_amplitude_data[i] = m_amplitude_data[j];
                m_phase_data[i] = m_phase_data[j];
            }
            else {
                m_amplitude_data[i] = 0;
                m_phase_data[i] = 0;
            }
        }

        m_amplitude_data[1] = 0;
        m_phase_data[1] = 0;

        return true;
    }


    bool Partials::interpolate(int32_t first_index, int32_t last_index, int16_t mode) noexcept {

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

        return true;
    }


    void Partials::normalizeAmplitude() noexcept {

        float max = maxAmplitude();

        if (max > std::numeric_limits<float>::min()) {
            float f = 1.0f / max;

            for (int32_t i = 0; i < m_resolution; i++) {
                m_amplitude_data[i] *= f;
            }
        }
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

        setAmplitudeAtIndex(0, 0);

        for (int32_t i = 1; i < m_resolution; i++) {
            double freq = root_freq * i;
            double pos = Freq::freqToPos(freq, root_freq, high_freq, 0, 1);
            float value = lut.lookup(static_cast<float>(pos));

            setAmplitudeAtIndex(i, Audio::amplitudeFromLevel(value * amount));
        }
    }


    void Partials::setByBezierValueCurve(BezierValueCurve* bezier_value_curve, int32_t sample_rate, float amount, float shift) noexcept {

        if (bezier_value_curve != nullptr) {
            float root_freq = static_cast<float>(sample_rate) / static_cast<float>(m_resolution) / 2;
            float high_freq = root_freq * static_cast<float>(m_resolution - 1);

            setAmplitudeAtIndex(0, 0.0f);

            for (int32_t i = 1; i < m_resolution; i++) {
                float freq = root_freq * i;
                freq = Audio::freqFromPitch(Audio::pitchFromFreq(static_cast<float>(freq)) + shift);
                double pos = Freq::freqToPos(freq, root_freq, high_freq, 0, 1);
                float value = bezier_value_curve->lookup(static_cast<float>(pos), 1024);
                setAmplitudeAtIndex(i, Audio::amplitudeFromLevel(value * amount));
                std::cout << i << ": " << Audio::amplitudeFromLevel(value * amount) << std::endl;
            }
        }
    }


    void Partials::setByEQ21(const EQ21& eq) noexcept {

        // TODO: Implement
    }


} // End of namespace Grain.
