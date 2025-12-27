//
//  Partials.hpp
//
//  Created by Roald Christesen on 24.03.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

#ifndef GrainPartials_hpp
#define GrainPartials_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Type/Range.hpp"
#include "Signal/Audio.hpp"


namespace Grain {

    class FreqBands;
    class BezierValueCurve;
    class EQ21;


    class PartialsRange {
    public:
        int32_t count_ = 0;
        float m_base_freq = 0.0f;
        float m_p_min = -1.0f;
        float m_p_max = -1.0f;
        int32_t m_min_bin_index = -1;
        int32_t m_max_bin_index = -1;
        float m_min_t = -1.0f;
        float m_max_t = -1.0f;
        float m_window_sum = 0.0f;

    public:
        PartialsRange() = default;

        [[nodiscard]] static const char* className() noexcept { return "PartialsRange"; }

        friend std::ostream& operator << (std::ostream& os, const PartialsRange* o) {
            o == nullptr ? os << "PartialsRange nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const PartialsRange& o) {
            os << o.m_min_bin_index << " to " << o.m_max_bin_index << " [" << o.width() << "]";
            os << ", t: " << o.m_min_t << " to " << o.m_max_t;
            os << ", window sum: " << o.m_window_sum;
            return os;
        }

        [[nodiscard]] int32_t width() const {
            if (m_min_bin_index < 0 || m_max_bin_index < 0) {
                return 0;
            }
            return m_max_bin_index - m_min_bin_index + 1;
        }

        void clear() {
            count_ = 0;
            m_base_freq = 0.0f;
            m_p_min = -1.0f;
            m_p_max = -1.0f;
            m_min_bin_index = -1;
            m_max_bin_index = -1;
            m_min_t = -1.0f;
            m_max_t = -1.0f;
            m_window_sum = 0.0f;
        }

        [[nodiscard]] float t(int32_t index) const;
        [[nodiscard]] float f(int32_t index) const;
    };


    /**
     *  @note Apple vDSP partials, DC to Nyquist frequency.
     */
    class Partials : public Object {
        friend class Signal;

    public:
        enum class Mode {
            Polar = 0,
            Cartesian = 1
        };

    public:
        explicit Partials(int32_t resolution) noexcept;
        explicit Partials(int32_t resolution, float* mem) noexcept;
        explicit Partials(const Partials* other) noexcept;
        ~Partials() noexcept override;

        [[nodiscard]] const char* className() const noexcept override { return "Partials"; }

        friend std::ostream& operator << (std::ostream& os, const Partials* o) {
            o == nullptr ? os << "Partials nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Partials& o) {
            os << "Partials resolution: " << o.m_resolution;
            os << ", use extern mem: " << o.m_use_extern_mem << std::endl;
            return os;
        }

        [[nodiscard]] int32_t resolution() const noexcept { return m_resolution; }
        [[nodiscard]] inline bool isBin(int32_t bin_index) const noexcept { return bin_index >= 0 && bin_index < m_resolution; }

        [[nodiscard]] bool hasData() const noexcept { return m_ma && m_ph; }
        [[nodiscard]] size_t memSize() const noexcept { return sizeof(float) * m_resolution * 2; }
        [[nodiscard]] float* mutMagPtr() const noexcept { return m_ma; }
        [[nodiscard]] float* mutPhasePtr() const noexcept { return m_ph; }

        [[nodiscard]] float dc() const noexcept { return m_dc; }
        [[nodiscard]] float mag(int32_t bin_index) const noexcept;
        [[nodiscard]] float phase(int32_t bin_index) const noexcept;

        [[nodiscard]] float magInterpolated(float bin_index) const noexcept;
        [[nodiscard]] float magLerp(float t) const noexcept;
        [[nodiscard]] float magMax() const noexcept;

        void clear() noexcept;
        void setDC(float dc) noexcept { m_dc = dc; }
        void setMag(int32_t bin_index, float mag) noexcept;
        void setMagNyquist(float mag) noexcept;
        void addMag(int32_t bin_index, float mag) noexcept;
        void scaleMag(int32_t bin_index, float mag) noexcept;
        void setPhase(int32_t bin_index, float phase) noexcept;
        bool set(const Partials* other) noexcept;
        void setByFormantData(int32_t n, const float* data, int32_t sample_rate) noexcept;

        bool accumulateProduct(const Partials* a, const Partials* b) noexcept;
        bool interpolate(int32_t first_bin_index, int32_t last_bin_index, int16_t mode) noexcept;

        void setByFreqRange(
                const FreqBands& freq_bands, int32_t sample_rate, float amount,
                float *lut_mem = nullptr, int32_t lut_length = 0) noexcept;
        void setByBezierValueCurve(
                BezierValueCurve* bezier_value_curve,
                int32_t sample_rate,
                float lo_hz, float hi_hz, float shift_hz = 0.0f) noexcept;
        void setByEQ21(const EQ21& eq) noexcept;

        [[nodiscard]] Partials* doubleResolution() noexcept;

        void rangeAtFreq(
                float base_freq, float freq, float cent_range,
                PartialsRange& out_range) const noexcept;

    protected:
        Mode m_mode = Mode::Cartesian;
        int32_t m_resolution = 0;       ///< Number of partials
        float m_dc = 0.0f;              ///< DC magnitude
        float* m_ma = nullptr;          ///< Partial magnitudes, highest partial at Nyquist frequency
        float* m_ph = nullptr;          ///< Partial phases, highest partial at Nyquist frequency
        bool m_use_extern_mem = false;  ///< `true` if memory is provided externally; `false if owned/allocated by this class
    };


} // End of namespace Grain

#endif // GrainPartials_hpp
