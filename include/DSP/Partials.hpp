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


namespace Grain {

    class FreqBands;
    class BezierValueCurve;
    class EQ21;


    class Partials : public Object {
        friend class Signal;

    public:
        enum class Mode {
            Cartesian,      ///< Values are real + imaginary
            Polar           ///< Values are amplitude and phase
        };

    private:
        Mode m_mode = Mode::Cartesian;
        int32_t m_resolution = 0;
        float* m_data_a = nullptr;
        float* m_data_b = nullptr;
        bool m_use_extern_mem = false;

    public:
        explicit Partials(int32_t resolution, Mode mode) noexcept;
        explicit Partials(int32_t resolution, float* mem, Mode mode) noexcept;
        Partials(const Partials& other) noexcept;
        ~Partials() noexcept override;

        [[nodiscard]] const char* className() const noexcept override { return "Partials"; }

        friend std::ostream& operator << (std::ostream& os, const Partials* o) {
            o == nullptr ? os << "Partials nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Partials& o) {
            os << "Partials, ";
            os << "m_mode: " << (o.m_mode == Mode::Cartesian ? "cartesian" : "polar");
            os << ", m_resolution: " << o.m_resolution;
            os << ", m_use_extern_mem: " << o.m_use_extern_mem << std::endl;
            return os;
        }

        void updateCartesian() noexcept;
        void updatePolar() noexcept;

        [[nodiscard]] bool isCartesian() const noexcept { return m_mode == Mode::Cartesian; }
        [[nodiscard]] bool isPolar() const noexcept { return m_mode == Mode::Polar; }
        [[nodiscard]] int32_t resolution() const noexcept { return m_resolution; }
        [[nodiscard]] bool hasData() const noexcept { return m_data_a; }
        [[nodiscard]] size_t memSize() const noexcept { return sizeof(float) * m_resolution * 2; }

        [[nodiscard]] float* mutRealData() const noexcept { return m_data_a; }
        [[nodiscard]] float* mutImagData() const noexcept { return m_data_b; }

        [[nodiscard]] bool isPartialIndex(int32_t index) const noexcept { return index >= 0 && index < m_resolution; }
        void valueAndPhaseAtIndex(int32_t index, float& out_amplitude, float& out_phase) const noexcept;
        [[nodiscard]] float amplitudeAtIndex(int32_t index) const noexcept;
        [[nodiscard]] float amplitudeLerpAtIndex(float index) const noexcept;
        [[nodiscard]] float amplitudeAtT(float t) const noexcept;
        void scaleAmplitudeAtIndex(int32_t index, float scale) noexcept;
        [[nodiscard]] float phaseAtIndex(int32_t index) const noexcept;
        [[nodiscard]] float maxAmplitude() const noexcept;


        void clear() noexcept;
        void clearAmplitudes() noexcept;
        void setAmplitudes(float value) noexcept;
        void clearPhases() noexcept;
        void setPhases(float value) noexcept;
        void sawtooth() noexcept;
        void square() noexcept;

        void setValueAtIndex(int32_t index, float value, int16_t mode) noexcept;
        void setPartialAtIndex(int32_t index, float amplitude, float phase) noexcept;
        void setPartialsInRange(int32_t start_index, int32_t end_index, float amplitude, float phase) noexcept;
        void setAllPartials(float amplitude, float phase) noexcept;
        void setAmplitudeAtIndex(int32_t index, float value) noexcept;
        void setAmplitudesInRange(int32_t start_index, int32_t end_index, float value) noexcept;
        void setAllAmplitudes(float value) noexcept;
        void setAllAmplitudes(const float* values) noexcept;
        void addAmplitudeAtIndex(int32_t index, float value) noexcept;
        void setPhaseAtIndex(int32_t index, float value) noexcept;
        void rotatePhaseAtIndex(int32_t index, float amount) noexcept;
        void setPhasesInRange(int32_t start_index, int32_t end_index, float value) noexcept;
        void setAllPhases(float value) noexcept;


        bool set(const Partials* other) noexcept;
        bool multiply(const Partials* other) noexcept;
        bool add(const Partials* other) noexcept;

        bool octaveUp() noexcept;
        bool interpolate(int32_t first_index, int32_t last_index, int16_t mode) noexcept;

        void normalizeAmplitude() noexcept;

        void setByFreqRange(const FreqBands& freq_bands, int32_t sample_rate, float amount, float *lut_mem = nullptr, int32_t lut_length = 0) noexcept;
        void setByBezierValueCurve(BezierValueCurve* bezier_value_curve, int32_t sample_rate, float amount = 1, float shift = 0) noexcept;
        void setByEQ21(const EQ21& eq) noexcept;
    };


    class PartialsRing {
    protected:
        int32_t m_size = 0;
        Partials::Mode m_partials_mode = Partials::Mode::Cartesian;
        int32_t m_partials_resolution = 0;
        Partials** m_partials_array = nullptr;

    public:
        explicit PartialsRing(int32_t size, int32_t partials_resolution, Partials::Mode partials_mode) {
            m_partials_mode = partials_mode;
            m_size = size;
            m_partials_resolution = partials_resolution;
            if (size > 0) {
                m_partials_array = static_cast<Partials**>(std::malloc(sizeof(Partials*) * size));
                if (m_partials_array) {
                    for (int32_t i = 0; i < size; ++i) {
                        m_partials_array[i] = nullptr;
                    }
                }
            }
        }
        ~PartialsRing() {
            if (m_partials_array) {
                for (int i = 0; i < m_size; ++i) {
                    if (m_partials_array[i]) {
                        delete m_partials_array[i];
                    }
                }
                std::free(m_partials_array);
            }
        }
        Partials* partialsAtIndex(int32_t index) {
            if (index < 0 || index >= m_size) {
                return nullptr;
            }

            if (!m_partials_array[index]) {
                m_partials_array[index] = new (std::nothrow) Partials(m_partials_resolution, m_partials_mode);
            }
            return m_partials_array[index];
        }
    };

} // End of namespace Grain

#endif // GrainPartials_hpp
