//
//  SignalAllPassFilter.hpp
//
//  Created by Roald Christesen on 16.09.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

//
//  GrFilterAllPass.cpp
//  GrainLib
//
//  Created by Roald Christesen on 16.09.15.
//

#include "Signal/SignalAllPassFilter.hpp"


namespace Grain {


    SignalAllPassFilter::SignalAllPassFilter(int32_t sampleRate) noexcept : SignalFilter(sampleRate) {

        SignalAllPassFilter::setFreq(8000.0f);
        SignalAllPassFilter::setFeedback(0.0f, false);
        SignalAllPassFilter::setStageCount(2);
        setInverted(false);

        SignalAllPassFilter::reset();
    }


    SignalAllPassFilter::~SignalAllPassFilter() noexcept {
    }


    void SignalAllPassFilter::setFreq(float freq) noexcept {

        m_freq = std::clamp<double>(freq, 10.0, _m_nyquist_freq);
        double a = (std::numbers::pi * m_freq) / m_sample_rate;

        double tan_a = std::tan(a);

        // Allpass coefficient: good range [-0.99, 0.99]
        _m_coef = (1.0f - tan_a) / (1.0f + tan_a);
        _m_coef = std::clamp<double>(_m_coef, -0.99, 0.99);
    }


    void SignalAllPassFilter::setStageCount(int32_t stage_count) noexcept {

        m_stage_count = std::clamp(stage_count, 2, kMaxStageCount);
        if (m_stage_count % 2 != 0) {
            m_stage_count++;  // ensure even
        }
    }


    void SignalAllPassFilter::setFeedback(float amount, bool phase_inverted) noexcept {

        m_feedback_amount = amount;
        m_feedback_phase_inverted = phase_inverted;
        _m_feedback_gain = phase_inverted ? -amount : amount;
        _m_feedback_enabled = amount > std::numeric_limits<float>::min();
    }


    void SignalAllPassFilter::reset() noexcept {

        for (int32_t i = 0; i < 10; i++) {
            _m_lx[i] = 0.0f;
            _m_ly[i] = 0.0f;
        }
    }


    float SignalAllPassFilter::process(float input) noexcept {

        float value = input;

        // Apply feedback from output of last stage
        if (_m_feedback_enabled) {
            value += _m_feedback_gain * _m_ly[m_stage_count - 1];
        }

        // Process all stages
        for (int32_t i = 1; i < m_stage_count; ++i) {
            float x = value;
            float y = _m_coef * (_m_ly[i] + x) - _m_lx[i];

            _m_lx[i] = x;
            _m_ly[i] = y;

            value = y;
        }

        // Output final stage result
        float output = m_inverted ? input - value : input + value;
        return std::isnan(output) ? 0.0f : output;
    }


} // End of namespace Grain
