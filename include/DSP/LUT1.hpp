//
//  LUT1.hpp
//
//  Created by Roald Christesen on 07.11.2013
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

#ifndef GrainLUT1_hpp
#define GrainLUT1_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Math/Math.hpp"
#include "DSP.hpp"


namespace Grain {


    class WeightedSamples;
    class Bezier;
    class LevelCurve;


    class LUT1 : public Object {

    public:

        enum {
            kMinResolution = 64,
            kMaxResolution = std::numeric_limits<int32_t>::max(),
            kDefaultResolution = 512,
            kMaxbezierResolution = 512,
            kDefaultBezierResolution = 64
        };

    private:
        int32_t m_resolution = 0;
        int32_t m_last_index = 0;
        int32_t m_max_resolution = 0;
        float* m_samples = nullptr;
        bool m_use_extern_mem = false;


    public:
        LUT1(int32_t resolution) noexcept;
        LUT1(int32_t resolution, int32_t max_resolution) noexcept;
        LUT1(int32_t resolution, float* mem) noexcept;
        ~LUT1() noexcept;

        void _init(int32_t resolution, int32_t max_resolution, float* mem) noexcept;

        ErrorCode setResolution(int32_t resolution) noexcept;

        int32_t resolution() const noexcept { return m_resolution; }
        const float* samplePtr() const noexcept { return m_samples; }
        float* mutSamplePtr() const noexcept { return m_samples; }
        const float* samplePtrAtIndex(int32_t index) const noexcept;
        float* mutSamplePtrAtIndex(int32_t index) const noexcept;

        void clear() noexcept;
        void clear(float value) noexcept;
        void scale(float scale) noexcept;
        void gamma(float e) noexcept;

        void add(const LUT1* lut) noexcept;
        void mul(const LUT1* lut) noexcept;

        void hanningWindow() noexcept;
        void window(DSP::WindowType window_type, float alpha, float beta, bool unity_gain) noexcept;

        void normalize() noexcept;
        void alignZero() noexcept;
        void limit(float min, float max) noexcept;
        void flip() noexcept;

        void setValueAtIndex(int32_t index, float value) noexcept;
        void setValuesInIndexRange(int32_t start_index, int32_t end_index, float value) noexcept;
        void setValuesInRange(float value, float t_left, float t_right) noexcept;
        void setEaseValuesInRange(Math::EaseMode ease_mode, float t_left, float t_right, float value_left, float value_right) noexcept;
        void setByBezier(const Bezier& bezier, int32_t bezier_resolution) noexcept;
        void setByLevelCurve(const LevelCurve& level_curve) noexcept;
        bool setByWeightedSamples(const WeightedSamples* weighted_samples) noexcept;
        bool setByGaussKernel(double sigma_sqr) noexcept;

        ErrorCode interpolateGaps(float* min = nullptr, float* max = nullptr) noexcept;

        void vibrato(int32_t sample_rate, int32_t n, int32_t offset, float amount, float freq, float delay, float attack, float duration, float release) noexcept;

        int32_t countAbove(float value) const noexcept;

        float lookup(float t) const noexcept;
        float firstValue() const noexcept;
        float lastValue() const noexcept;
        float valueAtIndex(int32_t index) const noexcept;
        float valueAtRealIndex(float real_index) const noexcept;

        inline int32_t tToIndex(float t) const noexcept { return static_cast<int32_t>(round(t * m_last_index)); };
    };



    class LUT1Stepper {

    private:
        LUT1* m_lut = nullptr;
        float m_pos = 0.0f;
        float m_step = 1.0f;
        float m_duration = 10.0f;
        float m_min = 0.0f;
        float m_max = 1.0f;
        bool m_running = true;

    public:
        LUT1Stepper(LUT1* lut, float duration, float step = 1) noexcept;

        void start() noexcept;
        bool next(float* out_value) noexcept;
        bool next(float step, float* out_value) noexcept;

        void setLUT(LUT1* lut);
        void setDuration(float duration) noexcept;
        void setDurationAndStep(float duration, float step) noexcept;
        void setRange(float min, float max) noexcept;
    };


} // End of namespace Grain

#endif // GrainLUT1_hpp
