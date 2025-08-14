//
//  SignalFilter.hpp
//
//  Created by Roald Christesen on 16.09.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 22.07.2025
//

#ifndef GrainSignalFilter_hpp
#define GrainSignalFilter_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"


namespace Grain {

    /**
     *  @brief Superclass for implementing DSP filters.
     *
     *  The SignalFilter class serves as a base class for implementing
     *  digital signal processing (DSP) filters. It encapsulates common
     *  functionality related to filter management, customization, and parameter changes.
     *
     *  A digital filter is a mathematical algorithm or software component
     *  used to modify or manipulate digital signals. It operates on
     *  discrete-time signals, represented as a sequence of discrete samples
     *  or values.
     *
     *  The primary purpose of a digital filter is to modify the
     *  characteristics of a signal by selectively attenuating or amplifying
     *  specific frequencies or frequency ranges.
     */
    class SignalFilter : public Object {
    public:
        explicit SignalFilter(int32_t sample_rate) noexcept {
            setSampleRate(sample_rate);
        }

        ~SignalFilter() noexcept override = default;


        [[nodiscard]] int32_t sampleRate() const noexcept { return m_sample_rate; }


        [[nodiscard]] virtual int32_t outputDelay() const noexcept { return 0; }
        virtual bool hasResonance() noexcept { return _m_has_resonance; }
        virtual bool hasFreqRange() noexcept { return _m_has_range; }
        [[nodiscard]] virtual float freq() const noexcept { return 1000.0f; }
        [[nodiscard]] virtual float feedback() const noexcept { return 0.0f; }
        [[nodiscard]] virtual int32_t stageCount() const noexcept { return 0; }

        [[nodiscard]] bool isInverted() const noexcept { return m_inverted; }
        [[nodiscard]] bool isValid() const noexcept { return m_valid; }

        void setSampleRate(int32_t sampleRate) noexcept {
            m_sample_rate = sampleRate < 1 ? 1 : sampleRate;
            _m_nyquist_freq = 0.5 * m_sample_rate;
            _m_pi_divided_by_sample_rate = std::numbers::pi / m_sample_rate;
        }
        virtual void setFreq(float freq) noexcept {}
        virtual void setFreq(float freq, float resonance) noexcept { setFreq(freq); }
        virtual void setFreqRange(float low_freq, float high_freq) noexcept {}
        virtual void setFeedback(float amount, bool phase_inverted) noexcept {};
        virtual void setStageCount(int32_t stage_count) noexcept {};

        void setInverted(bool inverted) noexcept { m_inverted = inverted; }

        /**
         *  @brief Resets the filter state for processing a new signal.
         *
         *  This method is intended to be implemented by derived classes.
         *  It should reset any internal state variables or buffers used by the filter.
         */
        virtual void reset() noexcept {}

        /**
         *  @brief Process a single sample through the filter.
         *
         *  This method is intended to be implemented by derived classes. It should be called
         *  sequentially for each sample to be processed.
         *
         *  @param input A single input sample to be filtered.
         *  @return The filtered output sample.
         */
        [[nodiscard]] virtual float process(float input) noexcept { return 0; }


        // Utilities

        [[nodiscard]] inline static float saturate(float input) {
            return 0.5f * (std::fabs(input + 0.95f) - std::fabs(input - 0.95f));
        }

        [[nodiscard]] inline static float saturate(float input, float
        threshold) {
            return 0.5 * (std::fabs(input + threshold) - std::fabs(input - threshold));
        }

        [[nodiscard]] inline static float limit(float input, float bound) {
            if (input < 0.0) {
                return -limit(-input, bound);
            }
            float v = bound - input;
            return bound - (v + std::fabs(v)) * 0.5;
        }

        [[nodiscard]] inline static float lerp(float a, float b, float f) {
            return (1.0f - f) * a + f * b;
        }

    protected:
        int32_t m_sample_rate = 44100;  ///< Samples per sesond, importent for many filter functions
        bool m_inverted = false;        ///< Determines whether the filtered output is subtracted or added to the original input
        bool m_valid = true;            ///< If set to true, the filter is invalid an must not be used

        double _m_pi_divided_by_sample_rate = std::numbers::pi / 44100.0;
        double _m_nyquist_freq = 22050.0;
        bool _m_has_resonance = false;  ///< If set to true, the filter has a resonance parameter
        bool _m_has_range = false;      ///< If set to true, the filter has a range parameter
    };


} // End of namespace Grain

#endif // GrainSignalFilter_hpp
