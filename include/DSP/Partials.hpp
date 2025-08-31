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
        enum {
            kModeAmplitude = 0,
            kModePhase
        };

    private:
        int32_t m_resolution = 0;
        float* m_amplitude_data = nullptr;
        float* m_phase_data = nullptr;
        size_t m_data_size = 0;
        bool m_use_extern_mem = false;

    public:
        Partials(int32_t resolution) noexcept;
        Partials(int32_t resolution, float* mem) noexcept;
        Partials(const Partials& partials) noexcept;
        virtual ~Partials() noexcept;

        [[nodiscard]] const char* className() const noexcept override { return "Partials"; }

        friend std::ostream& operator << (std::ostream& os, const Partials* o) {
            o == nullptr ? os << "Partials nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Partials& o) {
            os << "Partials:\n";
            os << "  m_resolution: " << o.m_resolution << std::endl;
            os << "  m_data_size: " << o.m_data_size << std::endl;
            os << "  m_use_extern_mem: " << o.m_use_extern_mem << std::endl;
            return os;
        }


        static int32_t dataSize(int32_t resolution) noexcept { return resolution * 2; }

        int32_t resolution() const noexcept { return m_resolution; }

        bool hasData() const noexcept { return m_amplitude_data; }
        bool isPartial(int32_t index) const noexcept { return index >= 0 && index < m_resolution; }

        void partialAtIndex(int32_t index, float& out_amplitude, float& out_phase) const noexcept;
        float amplitudeAtIndex(int32_t index) const noexcept;
        float amplitudeLerpAtIndex(float index) const noexcept;
        float amplitudeAtNormalizedIndex(float t) const noexcept;
        void scaleAmplitudeAtIndex(int32_t index, float scale) noexcept;
        float phaseAtIndex(int32_t index) const noexcept;
        float maxAmplitude() const noexcept;

        float valueAtIndex(int32_t index, int16_t mode) const noexcept;


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


        bool set(const Partials* partials) noexcept;
        bool mul(const Partials* partials) noexcept;
        bool complexMultiply(const Partials* partials) noexcept;
        bool add(const Partials* partials) noexcept;
        bool octaveUp() noexcept;
        bool interpolate(int32_t first_index, int32_t last_index, int16_t mode) noexcept;

        void normalizeAmplitude() noexcept;

        void setByFreqRange(const FreqBands& freq_bands, int32_t sample_rate, float amount, float *lut_mem = nullptr, int32_t lut_length = 0) noexcept;
        void setByBezierValueCurve(BezierValueCurve* bezier_value_curve, int32_t sample_rate, float amount = 1, float shift = 0) noexcept;
        void setByEQ21(const EQ21& eq) noexcept;
    };


} // End of namespace Grain

#endif // GrainPartials_hpp
