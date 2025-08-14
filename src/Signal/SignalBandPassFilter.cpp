//
//  SignalBandPassFilter.cpp
//
//  Created by Roald Christesen on 07.09.2017
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Signal/SignalBandPassFilter.hpp"


namespace Grain {


    SignalBandPassFilter::SignalBandPassFilter(int32_t sample_rate) noexcept : SignalFilter(sample_rate) {

        setFilterType(FilterType::BandPass1);
        SignalBandPassFilter::reset();
        SignalBandPassFilter::setFreq(100.0f, 200.0f);
    }


    SignalBandPassFilter::SignalBandPassFilter(int32_t sample_rate, FilterType filterType, float lowFreq, float highFreq) noexcept : SignalFilter(sample_rate) {

        setFilterType(filterType);
        SignalBandPassFilter::reset();
        SignalBandPassFilter::setFreqRange(lowFreq, highFreq);
    }


    SignalBandPassFilter::~SignalBandPassFilter() noexcept {
    }


    const char *SignalBandPassFilter::filterTypeName() const noexcept {

        switch (m_filter_type) {
            case FilterType::BandPass1: return "band pass 1";
        }

        return "unknown";
    }


    void SignalBandPassFilter::setFilterType(FilterType filter_type) noexcept {

        _m_has_range = true;
        m_filter_type = filter_type;
        reset();
    }


    void SignalBandPassFilter::setFreqRange(float low_freq, float high_freq) noexcept {

        m_low_freq = std::clamp<double>(low_freq, 20.0, 9999.0);
        m_high_freq = std::clamp<double>(high_freq, m_low_freq * 1.001, 10000.0);

        switch (m_filter_type) {
            case FilterType::BandPass1:
            {
                double bw = (m_high_freq - m_low_freq);
                double cf = bw * 0.5 + m_low_freq;
                double ny = m_sample_rate * 0.49;
                if (cf < 20.0) {
                    cf = 20;
                }
                else if (cf > ny) {
                    cf = ny;
                }

                _m_b2 = std::exp(-2 * _m_pi_divided_by_sample_rate * bw);
                _m_b1 = (-4.0 * _m_b2) / (1.0 + _m_b2) * std::cos(std::numbers::pi * cf / m_sample_rate);
                _m_a1 = 1.0 - std::sqrt(_m_b2);
            }
                break;

            default:
                break;
        }
    }


    void SignalBandPassFilter::reset() noexcept {

        _m_x1 = _m_x2 = _m_y1 = _m_y2 = 0.0;
    }


    float SignalBandPassFilter::process(float input) noexcept {

        float output = input;

        switch (m_filter_type) {
            case FilterType::BandPass1:
            {
                double temp = (_m_a1 * input) - (_m_a1 * _m_x2) - (_m_b1 * _m_y1) - (_m_b2 * _m_y2);
                _m_x2 = _m_x1;
                _m_x1 = input;
                _m_y2 = _m_y1;
                _m_y1 = temp;
                output = temp;
            }
                break;
        }

        if (m_inverted) {
            return input - output;
        }
        else {
            return output;
        }
    }


} // End of namespace Grain
