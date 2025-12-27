//
//  SignalFormantFilter.cpp
//
//  Created by Roald Christesen on 21.04.2017
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Signal/SignalFormantFilter.hpp"
#include "Signal/Audio.hpp"


namespace Grain {


SignalFormantFilter::SignalFormantFilter(int32_t sample_rate) noexcept : SignalFilter(sample_rate) {
    for (int32_t i = 0; i < kMaxFormantCount; i++) {
        m_filter[i] = new(std::nothrow) SignalButterworthFilter(sample_rate);
        m_filter[i]->setFilterType(SignalButterworthFilter::FilterType::BandPass);
    }
}


SignalFormantFilter::~SignalFormantFilter() noexcept {
    for (int32_t i = 0; i < kMaxFormantCount; i++) {
        delete m_filter[i];
    }
}


ErrorCode SignalFormantFilter::setByData(int32_t formant_count, float* data) noexcept {
    if (data == nullptr) {
        return ErrorCode::NullPointer;
    }

    auto n = std::clamp<int32_t>(formant_count, 0, kMaxFormantCount);
    auto d = data;
    for (int32_t i = 0; i < n; i++) {
        m_freq[i] = d[i * 3 + 0];
        m_amp[i] = Audio::dbToLinear(d[i * 3 + 1]);
        m_bw_cent[i] = d[i * 3 + 2];

        float c = m_bw_cent[i] * 0.01f;
        auto pitch = Audio::pitchFromFreq(m_freq[i]);
        m_lo_freq[i] = Audio::freqFromPitch(pitch - c);
        m_hi_freq[i] = Audio::freqFromPitch(pitch + c);
        m_filter[i]->setFreqRange(m_lo_freq[i], m_hi_freq[i]);
    }

    return ErrorCode::None;
}


void SignalFormantFilter::reset() noexcept {
    std::cout << this << std::endl;
    for (int32_t i = 0; i < kMaxFormantCount; i++) {
        m_filter[i]->reset();
    }
}


float SignalFormantFilter::process(float input) noexcept {
    float output = 0.0f;
    for (int32_t i = 0; i < kMaxFormantCount; i++) {
        output += m_filter[i]->process(input) * m_amp[i];
    }

    return output;
}


} // End of namespace Grain
