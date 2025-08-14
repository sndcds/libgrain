//
//  SignalLowPassFilter.cpp
//
//  Created by Roald Christesen on 09.09.22015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Signal/SignalLowPassFilter.hpp"
#include "Type/Type.hpp"


namespace Grain {


    SignalLowPassFilter::SignalLowPassFilter(int32_t sample_rate) noexcept : SignalFilter(sample_rate) {

        _m_has_resonance = true;
        SignalLowPassFilter::setFreq(8000.0f);
        SignalLowPassFilter::reset();
    }


    void SignalLowPassFilter::setFreq(float freq, float resonance) noexcept {

        m_freq = std::clamp<float>(freq, 20.0f, 15000.0f);
        m_resonance = std::clamp<float>(resonance, 0.0f, 1.0f);

        double c = (m_freq + m_freq) / m_sample_rate;
        _m_p = c * (1.8 - 0.8 * c);
        _m_k = 2.0 * std::sin(c * std::numbers::pi * 0.5) - 1.0;
        double t1 = (1.0 - _m_p) * 1.386249;
        double t2 = 12.0 + t1 * t1;
        double t1_6 = 6.0 * t1;

        _m_q = m_resonance * (t2 + t1_6) / (t2 - t1_6) * 0.65;
    }


    void SignalLowPassFilter::reset() noexcept {

        memset(_m_stage, 0, sizeof(_m_stage));
        memset(_m_delay, 0, sizeof(_m_delay));
    }


    float SignalLowPassFilter::process(float input) noexcept {

        double x = input - _m_q * _m_stage[3];

        // Four cascaded one-pole filters (bilinear transform)
        _m_stage[0] = x * _m_p + _m_delay[0] * _m_p - _m_k * _m_stage[0];
        _m_stage[1] = _m_stage[0] * _m_p + _m_delay[1] * _m_p - _m_k * _m_stage[1];
        _m_stage[2] = _m_stage[1] * _m_p + _m_delay[2] * _m_p - _m_k * _m_stage[2];
        _m_stage[3] = _m_stage[2] * _m_p + _m_delay[3] * _m_p - _m_k * _m_stage[3];

        // Clipping band-limited sigmoid
        _m_stage[3] -= (_m_stage[3] * _m_stage[3] * _m_stage[3]) / 6.0;

        _m_delay[0] = x;
        _m_delay[1] = _m_stage[0];
        _m_delay[2] = _m_stage[1];
        _m_delay[3] = _m_stage[2];

        if (m_inverted) {
            return -static_cast<float>(_m_stage[3]);
        }
        else {
            return static_cast<float>(_m_stage[3]);
        }
    }

} // End of namespace Grain
