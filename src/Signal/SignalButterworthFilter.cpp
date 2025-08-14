//
//  SignalButterworthFilter.cpp
//
//  Created by Roald Christesen on 09.06.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Signal/SignalButterworthFilter.hpp"


namespace Grain {


    SignalButterworthFilter::SignalButterworthFilter(int32_t sample_rate) noexcept : SignalFilter(sample_rate) {

        setFilterType(FilterType::LowPass);
        SignalButterworthFilter::reset();
        SignalButterworthFilter::setFreq(1000);
    }


    SignalButterworthFilter::SignalButterworthFilter(int32_t sample_rate, FilterType filter_type, float freq) noexcept : SignalFilter(sample_rate) {

        setFilterType(filter_type);
        SignalButterworthFilter::reset();
        SignalButterworthFilter::setFreq(freq);
    }


    SignalButterworthFilter::~SignalButterworthFilter() noexcept {
    }


    const char *SignalButterworthFilter::filterTypeName() const noexcept {

        switch (m_filter_type) {
            case FilterType::LowPass: return "low pass";
            case FilterType::HighPass: return "high pass";
            case FilterType::BandPass: return "band pass";
            case FilterType::BandStop: return "band stop";
        }

        return "unknown";
    }


    void SignalButterworthFilter::setFilterType(FilterType filter_type) noexcept {

        m_filter_type = filter_type;

        _m_has_range = false;
        switch (filter_type) {
            case FilterType::BandPass:
            case FilterType::BandStop:
                _m_has_range = true;
                break;

            default:
                break;
        }

        m_r = 0.0;
        m_s = 0.0;
        m_a = 0.0;
        m_d1 = 0.0;
        m_d2 = 0.0;
        m_d3 = 0.0;
        m_d4 = 0.0;
    }


    void SignalButterworthFilter::setFreq(float freq) noexcept {

        m_freq = std::clamp<double>(freq, 20.0, 10000.0);

        double a = std::tan(_m_pi_divided_by_sample_rate * m_freq);
        double a2 = a * a;
        double s = a2 + a * 2 * M_SQRT1_2 + 1;

        switch (m_filter_type) {
            case FilterType::LowPass:
                m_a = a2 / s;
                m_d1 = 2.0 * (1.0 - a2) / s;
                m_d2 = -(a2 - a * 2 * M_SQRT1_2 + 1) / s;
                return;

            case FilterType::HighPass:
                m_a = 1.0 / s;
                m_d1 = 2.0 * (1.0 - a2) / s;
                m_d2 = -(a2 - a * 2 * M_SQRT1_2 + 1 ) / s;
                return;

            default:
                break;
        }
    }


    void SignalButterworthFilter::setFreqRange(float low_freq, float high_freq) noexcept {

        m_low_freq = std::clamp<double>(low_freq, 20.0f, 9999.0f);
        m_high_freq = std::clamp<double>(high_freq, m_low_freq * 1.001f, 10000.0f);


        double p = _m_pi_divided_by_sample_rate * (m_high_freq - m_low_freq);

        double a = std::cos(_m_pi_divided_by_sample_rate * (m_high_freq + m_low_freq)) / std::cos(p);
        double a2 = a * a;
        double b = tan(p);
        double b2 = b * b;

        double _v1 = b * M_SQRT1_2;
        double _v2 = _v1 + _v1;
        double _v3 = a * 4;

        double s = 1.0 / (b2 + _v2 + 1.0);

        switch (m_filter_type) {
            case FilterType::BandPass:
                m_a = b2 * s;
                m_d1 = _v3 * (1.0 + _v1) * s;
                m_d2 = 2.0 * (b2 - (a2 + a2) - 1.0) * s;
                m_d3 = _v3 * (1.0 - _v1) * s;
                m_d4 = -(b2 - _v2 + 1.0) * s;
                break;

            case FilterType::BandStop:
                m_a = s;
                m_d1 = _v3 * (1.0 + _v1) * s;
                m_d2 = 2.0 * (b2 - (a2 + a2) - 1.0) * s;
                m_d3 = _v3 * (1.0 - _v1) * s;
                m_d4 = -(b2 - _v2 + 1.0) * s;
                m_r = _v3;
                m_s = 4.0 * a2 + 2.0;
                break;

            default:
                break;
        }
    }


    void SignalButterworthFilter::reset() noexcept {

        m_w[0] = 0.0;
        m_w[1] = 0.0;
        m_w[2] = 0.0;
        m_w[3] = 0.0;
        m_w[4] = 0.0;
    }


    float SignalButterworthFilter::process(float input) noexcept {

        float output;

        switch (m_filter_type) {
            case FilterType::LowPass:
                m_w[0] = m_d1 * m_w[1] + m_d2 * m_w[2] + input;
                output = m_a * (m_w[0] + (m_w[1] + m_w[1]) + m_w[2]);
                m_w[2] = m_w[1];
                m_w[1] = m_w[0];
                break;

            case FilterType::HighPass:
                m_w[0] = m_d1 * m_w[1] + m_d2 * m_w[2] + input;
                output = m_a * (m_w[0] - (m_w[1] + m_w[1]) + m_w[2]);
                m_w[2] = m_w[1];
                m_w[1] = m_w[0];
                break;

            case FilterType::BandPass:
            {
                m_w[0] = m_d1 * m_w[1] + m_d2 * m_w[2] + m_d3 * m_w[3] + m_d4 * m_w[4] + input;
                output = m_a * (m_w[0] - (m_w[2] + m_w[2]) + m_w[4]);
                m_w[4] = m_w[3];
                m_w[3] = m_w[2];
                m_w[2] = m_w[1];
                m_w[1] = m_w[0];
            }
                break;

            case FilterType::BandStop:
            {
                m_w[0] = m_d1 * m_w[1] + m_d2 * m_w[2] + m_d3 * m_w[3] + m_d4 * m_w[4] + input;
                output = m_a * (m_w[0] - m_r * m_w[1] + m_s * m_w[2]- m_r * m_w[3] + m_w[4]);
                m_w[4] = m_w[3];
                m_w[3] = m_w[2];
                m_w[2] = m_w[1];
                m_w[1] = m_w[0];
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
