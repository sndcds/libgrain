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

    Partials::Partials(int32_t resolution, Mode mode) noexcept : Object() {
        m_mode = mode;
        m_resolution = resolution;
        m_primary = static_cast<float*>(std::malloc(memSize()));
        m_secondary = _secondary();
        m_use_extern_mem = false;
        clear();
    }


    Partials::Partials(int32_t resolution, float* mem, Mode mode) noexcept : Object() {
        m_mode = mode;
        m_resolution = resolution;
        m_primary = mem;
        m_secondary = _secondary();
        m_use_extern_mem = true;
        clear();
    }


    Partials::Partials(const Partials& other) noexcept : Object() {
        m_mode = other.m_mode;
        m_resolution = other.m_resolution;
        m_primary = static_cast<float*>(std::malloc(memSize()));
        m_secondary = _secondary();
        if (m_primary) {
            memcpy(m_primary, other.m_primary, memSize());
        }
        m_use_extern_mem = false;
    }


    Partials::~Partials() noexcept {
        if (m_primary && !m_use_extern_mem) {
            std::free(m_primary);
        }
    }


    void Partials::updateCartesian() noexcept {
        if (m_mode != Mode::Cartesian) {
#if defined(__APPLE__) && defined(__MACH__)
            m_primary[0] = m_primary[0] * cosf(m_secondary[0]); // DC -> realp[0]
            m_secondary[0] = m_primary[m_resolution]; // Nyquist -> imagp[0]
            for (int i = 1; i < m_resolution; ++i) {
                float mag = m_primary[i];
                float phase = m_secondary[i];
                m_primary[i]   = mag * cosf(phase); // realp[i]
                m_secondary[i] = mag * sinf(phase); // imagp[i]
            }
#else
            for (int i = 0; i <= m_resolution; ++i) {
                float mag = m_primary[i];
                float phase = m_secondary[i];
                m_primary[i] = mag * cosf(phase);
                m_secondary[i] = mag * sinf(phase);
            }
#endif
            m_mode = Mode::Cartesian;
        }
    }


    void Partials::updatePolar() noexcept {
        if (m_mode != Mode::Polar) {
#if defined(__APPLE__) && defined(__MACH__)
            // DC (bin 0)
            float re0 = m_primary[0]; // realp[0]
            m_primary[0] = sqrtf(re0 * re0); // DC magnitude, no phase
            m_secondary[0] = 0.0f; // DC phase, allways 0
            // Nyquist (bin N/2), stored in imagp[0]
            float re_ny = m_secondary[0]; // Nyquist has no real part in vDSP
            m_primary[m_resolution] = sqrtf(re_ny * re_ny); // Nyquist magnitude, no phase
            m_secondary[m_resolution] = 0.0f;  //  Nyquist phase, allways 0
            // bins 1..N/2-1
            for (int i = 1; i < m_resolution; ++i) {
                float re = m_primary[i]; // realp[i]
                float im = m_secondary[i]; // imagp[i]
                m_primary[i] = sqrtf(re * re + im * im); // magnitude
                m_secondary[i] = atan2f(im, re); // phase
            }
#else
            // FFTW: all bins explicit
            for (int i = 0; i <= m_resolution; ++i) {
                float re = m_primary[i];
                float im = m_secondary[i];
                m_primary[i]   = sqrtf(re*re + im*im);  // magnitude
                m_secondary[i] = atan2f(im, re);        // phase
            }
#endif
            m_mode = Mode::Polar;
        }
    }


    void Partials::valueAndPhaseAtIndex(int32_t index, float& out_amplitude, float& out_phase) const noexcept {
        if (isPartialIndex(index)) {
            if (m_mode == Mode::Cartesian) {
                float re = m_primary[index];
                float im = m_secondary[index];
                out_amplitude = sqrtf(re * re + im * im);
                out_phase = atan2f(im, re);
            }
            else {
                out_amplitude = m_primary[index];
                out_phase = m_secondary[index];
            }
        }
    }


    float Partials::amplitudeAtIndex(int32_t index) const noexcept {
        if (isPartialIndex(index)) {
            if (m_mode == Mode::Cartesian) {
                float re = m_primary[index];
                float im = m_secondary[index];
                return sqrtf(re * re + im * im);
            }
            else {
                return m_primary[index];
            }
        }
        return 0.0f;
    }


    float Partials::amplitudeLerpAtIndex(float index) const noexcept {
        if (index <= 0.0f) {
            return amplitudeAtIndex(0);
        }

        if (index >= static_cast<float>(m_resolution - 1)) {
            return amplitudeAtIndex(m_resolution - 1);
        }

        auto int_index = static_cast<int32_t>(index);
        float f1 = index - static_cast<float>(int_index);
        float f0 = 1.0f - f1;

        return amplitudeAtIndex(int_index) * f0 + amplitudeAtIndex(int_index + 1) * f1;
    }


    float Partials::amplitudeAtT(float t) const noexcept {
        return amplitudeLerpAtIndex(t * static_cast<float>(m_resolution - 1));
    }


    void Partials::scaleAmplitudeAtIndex(int32_t index, float scale) noexcept {
        if (isPartialIndex(index)) {
            updatePolar();
            m_primary[index] *= scale;
        }
    }


    float Partials::phaseAtIndex(int32_t index) const noexcept {
        if (isPartialIndex(index)) {
            if (m_mode == Mode::Cartesian) {
                float re = m_primary[index];
                float im = m_secondary[index];
                return atan2f(im, re);
            }
            else {
                return m_secondary[index];
            }
        }
        return 0.0f;
    }


    void Partials::clear() noexcept {
        if (hasData()) {
            memset(m_primary, 0, memSize());
        }
    }


    void Partials::clearAmplitudes() noexcept {
        setAmplitudes(0);
    }


    void Partials::setAmplitudes(float value) noexcept {
        if (hasData()) {
            updatePolar();
            for (int32_t i = 0; i < m_resolution; i++) {
                m_primary[i] = value;
            }
        }
    }


    void Partials::clearPhases() noexcept {
        setPhases(0);
    }


    void Partials::setPhases(float value) noexcept {
        if (hasData()) {
            updatePolar();
            for (int32_t i = 0; i < m_resolution; i++) {
                m_secondary[i] = value;
            }
        }
    }


    void Partials::sawtooth() noexcept {
        if (hasData()) {
            m_mode = Mode::Polar;
            m_primary[0] = 0.0f;
            m_secondary[0] = 0.0f;
            for (int32_t i = 1; i < m_resolution; i++) {
                m_primary[i] = 1.0f / i;
                m_secondary[i] = 0.0f;
            }
        }
    }


    void Partials::square() noexcept {
        if (hasData()) {
            m_mode = Mode::Polar;
            m_primary[0] = 1.0f;
            m_secondary[0] = 0.0f;
            for (int32_t i = 1; i < m_resolution; i++) {
                m_primary[i] = i % 2 ? 1 / i : 0;
                m_secondary[i] = 0.0f;
            }
        }
    }


    float Partials::maxAmplitude() const noexcept {
        float max = 0;
        if (m_mode == Mode::Polar) {
            for (int32_t i = 0; i < m_resolution; i++) {
                float a = m_primary[i];
                if (a > max) {
                    max = a;
                }
            }
        }
        else {
            for (int32_t i = 0; i < m_resolution; i++) {
                float a = amplitudeAtIndex(i);
                if (a > max) {
                    max = a;
                }
            }
        }
        return max;
    }


    void Partials::setPartialAtIndex(int32_t index, float amplitude, float phase) noexcept {
        if (isPartialIndex(index)) {
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
        if (isPartialIndex(index)) {
            updatePolar();
            m_primary[index] = value;
        }
    }


    void Partials::setAmplitudesInRange(int32_t start_index, int32_t end_index, float value) noexcept {
        start_index = std::clamp(start_index, 0, m_resolution - 1);
        end_index = std::clamp(end_index, start_index, m_resolution - 1);
        updatePolar();
        for (int32_t i = start_index; i <= end_index; i++) {
            m_primary[i] = value;
        }
    }


    void Partials::setAllAmplitudes(float value) noexcept {
        updatePolar();
        for (int32_t i = 0; i < m_resolution; i++) {
            m_primary[i] = value;
        }
    }


    void Partials::setAllAmplitudes(const float* values) noexcept {
        if (values) {
            updatePolar();
            for (int32_t i = 0; i < m_resolution; i++) {
                m_primary[i] = values[i];
            }
        }
    }


    void Partials::addAmplitudeAtIndex(int32_t index, float value) noexcept {
        /* TODO: !!!!
        if (isPartial(index)) {
            m_amplitude_data[index] = m_amplitude_data[index] + value;
        }
         */
    }


    void Partials::setPhaseAtIndex(int32_t index, float value) noexcept {
        /* TODO: !!!!
        if (isPartial(index)) {
            m_phase_data[index] = Type::wrappedBipolarPI<float>(value);
        }
         */
    }


    void Partials::rotatePhaseAtIndex(int32_t index, float amount) noexcept {
        /* TODO: !!!!
        if (isPartial(index)) {
            m_phase_data[index] = Type::wrappedBipolarPI<float>(m_phase_data[index] + amount);
        }
         */
    }


    void Partials::setPhasesInRange(int32_t start_index, int32_t end_index, float value) noexcept {
        start_index = std::clamp(start_index, 0, m_resolution - 1);
        end_index = std::clamp(end_index, start_index, m_resolution - 1);
        value = Type::wrappedBipolarPI<float>(value);
        /* TODO: !!!!
        for (int32_t i = start_index; i <= end_index; i++) {
            m_phase_data[i] = value;
        }
         */
    }


    void Partials::setAllPhases(float value) noexcept {
        value = std::clamp(value, static_cast<float>(-std::numbers::pi), static_cast<float>(std::numbers::pi));
        /* TODO: !!!!
        for (int32_t i = 0; i < m_resolution ; i++) {
            m_phase_data[i] = value;
        }
         */
    }


    bool Partials::set(const Partials* other) noexcept {
        if (other && other->m_resolution == m_resolution && hasData() && other->hasData()) {
            memcpy(m_primary, other->m_primary, memSize());
            m_mode = other->m_mode;
            return true;
        }
        else {
            return false;
        }
    }


    bool Partials::multiply(const Partials* other) noexcept {
        if (other &&
            other->m_mode == Partials::Mode::Cartesian && m_mode == Partials::Mode::Cartesian &&
            other->m_resolution == m_resolution &&
            hasData() && other->hasData())
        {
            float* ar = m_primary;
            float* ai = m_secondary;
            float* br = other->m_primary;
            float* bi = other->m_secondary;

            for (int32_t i = 0; i < m_resolution; i++) {
                float rr =  (ar[i] * br[i]) + (ai[i] * bi[i] * -1.0f);
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


    bool Partials::add(const Partials* other) noexcept {
        if (other &&
            other->m_mode == Partials::Mode::Cartesian && m_mode == Partials::Mode::Cartesian &&
            other->m_resolution == m_resolution &&
            hasData() && other->hasData())
        {
            for (int i = 0; i < m_resolution; ++i) {
                m_primary[i] += other->m_primary[i];
                m_secondary[i] += other->m_secondary[i];
            }
            return true;
        }
        else {
            return false;
        }
    }


    bool Partials::octaveUp() noexcept {
        // TODO: Implement!
        /*
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
        */
        return false;
    }


    bool Partials::interpolate(int32_t first_index, int32_t last_index, int16_t mode) noexcept {
        /* TODO: !!!!
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


    void Partials::normalizeAmplitude() noexcept {
        float max = maxAmplitude();
        /* TODO: !!!!
        if (max > std::numeric_limits<float>::min()) {
            float f = 1.0f / max;

            for (int32_t i = 0; i < m_resolution; i++) {
                m_amplitude_data[i] *= f;
            }
        }
         */
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


    void Partials::setByBezierValueCurve(
            BezierValueCurve* bezier_value_curve,
            int32_t sample_rate,
            float amount,
            float shift
    ) noexcept
    {
        if (bezier_value_curve) {
            float root_freq = static_cast<float>(sample_rate) / static_cast<float>(m_resolution) / 2;
            float high_freq = root_freq * static_cast<float>(m_resolution - 1);

            setAmplitudeAtIndex(0, 0.0f);

            for (int32_t i = 1; i < m_resolution; i++) {
                float freq = root_freq * i;
                freq = Audio::freqFromPitch(Audio::pitchFromFreq(static_cast<float>(freq)) + shift);
                float pos = static_cast<float>(Freq::freqToPos(freq, root_freq, high_freq, 0, 1));
                float value = bezier_value_curve->lookup(pos, 1024);
                setAmplitudeAtIndex(i, Audio::amplitudeFromLevel(value * amount));
                std::cout << i << ": " << Audio::amplitudeFromLevel(value * amount) << std::endl;
            }

            std::cout << "Partial: " << this << std::endl;
            for (int32_t i = 0; i < m_resolution; i++) {
                std::cout << i << " ... " << m_primary[i] << " / " << m_secondary[i] << std::endl;
            }
        }
    }


    void Partials::setByEQ21(const EQ21& eq) noexcept {
        // TODO: Implement
    }


} // End of namespace Grain
