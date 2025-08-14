//
//  SignalLadderFilter.cpp
//
//  Created by Roald Christesen on 30.09.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Signal/SignalLadderFilter.hpp"


namespace Grain {


    SignalLadderFilter::SignalLadderFilter(int32_t sample_rate) noexcept : SignalFilter(sample_rate) {

        m_drive = 0.0f;
        _m_has_resonance = true;
        _m_ipi = 4.0 * std::atan(1.0);

        SignalLadderFilter::setFreq(1000.0f, 0.0f); 	// normalized cutoff frequency
        SignalLadderFilter::reset();
    }


    void SignalLadderFilter::setFreq(float freq, float resonance) noexcept {

        std::cout << "SignalLadderFilter::setFreq: " << freq << ", resonance: " << resonance << std::endl;
        m_freq = std::clamp<double>(freq, 10.0f, 12000.0f);

        _m_kf =  m_freq / m_sample_rate;
        _m_kfc = _m_kf * 0.5;

        // Frequency & amplitude correction
        _m_kfcr = 1.8730 * (_m_kfc * _m_kfc * _m_kfc) + 0.4955 * (_m_kfc * _m_kfc) - 0.6490 * _m_kfc + 0.9988;
        _m_kacr = -3.9364 * (_m_kfc * _m_kfc) + 1.8409 * _m_kfc + 0.9968;
        _m_k2vg = _m_i2v * (1.0 - std::exp(-2 * _m_ipi * _m_kfcr * _m_kf));  // Filter tuning

        m_resonance = std::clamp<double>(resonance, 0.0, 1.0) * 0.8f;  // Filter becomes unstable above 0.8

        std::cout << "m_drive: " << m_drive << std::endl;
        std::cout << "_m_ipi: " << _m_ipi << std::endl;
        std::cout << "_m_kfc: " << _m_kfc << std::endl;
        std::cout << "_m_kf: " << _m_kf << std::endl;
        std::cout << "_m_kfcr: " << _m_kfcr << std::endl;
        std::cout << "_m_kacr: " << _m_kacr << std::endl;
        std::cout << "_m_k2vg: " << _m_k2vg << std::endl;
        std::cout << "_m_az1: " << _m_az1 << std::endl;
        std::cout << "_m_az2: " << _m_az2 << std::endl;
        std::cout << "_m_az3: " << _m_az3 << std::endl;
        std::cout << "_m_az4: "  << _m_az4 << std::endl;
        std::cout << "_m_az5: " << _m_az5 << std::endl;
        std::cout << "_m_ay1: " << _m_ay1 << std::endl;
        std::cout << "_m_ay2: " << _m_ay2 << std::endl;
        std::cout << "_m_ay3: " << _m_ay3 << std::endl;
        std::cout << "_m_ay4: " << _m_ay4 << std::endl;
        std::cout << "_m_amf: " << _m_amf << std::endl;

    }


    void SignalLadderFilter::reset() noexcept {

        _m_az1 = 0.0;
        _m_az2 = 0.0;
        _m_az3 = 0.0;
        _m_az4 = 0.0;
        _m_az5 = 0.0;
        _m_ay4 = 0.0;
        _m_amf = 0.0;
    }


    float SignalLadderFilter::process(float input) noexcept {

        // Cascade of 4 1st order sections
        _m_ay1 = _m_az1 + _m_k2vg * (std::tanh((input - 4 * m_resonance * _m_amf * _m_kacr) / _m_i2v) - std::tanh(_m_az1 / _m_i2v));
        _m_az1 = _m_ay1;
        _m_ay2 = _m_az2 + _m_k2vg * (std::tanh(_m_ay1 / _m_i2v) - std::tanh(_m_az2 / _m_i2v));
        _m_az2 = _m_ay2;
        _m_ay3 = _m_az3 + _m_k2vg * (std::tanh(_m_ay2 / _m_i2v) - std::tanh(_m_az3 / _m_i2v));
        _m_az3 = _m_ay3;
        _m_ay4 = _m_az4 + _m_k2vg * (std::tanh(_m_ay3 / _m_i2v) - std::tanh(_m_az4 / _m_i2v));
        _m_az4 = _m_ay4;

        // 1/2-sample delay for phase compensation
        _m_amf = (_m_ay4 + _m_az5) * 0.5;
        _m_az5 = _m_ay4;

        // Oversampling
        _m_ay1 = _m_az1 + _m_k2vg * (std::tanh((input - 4 * m_resonance * _m_amf * _m_kacr) / _m_i2v) - std::tanh(_m_az1 / _m_i2v));
        _m_az1 = _m_ay1;
        _m_ay2 = _m_az2 + _m_k2vg * (std::tanh(_m_ay1 / _m_i2v) - std::tanh(_m_az2 / _m_i2v));
        _m_az2 = _m_ay2;
        _m_ay3 = _m_az3 + _m_k2vg * (std::tanh(_m_ay2 / _m_i2v) - std::tanh(_m_az3 / _m_i2v));
        _m_az3 = _m_ay3;
        _m_ay4 = _m_az4 + _m_k2vg * (std::tanh(_m_ay3 / _m_i2v) - std::tanh(_m_az4 / _m_i2v));
        _m_az4 = _m_ay4;
        _m_amf = (_m_ay4 + _m_az5) * 0.5;
        _m_az5 = _m_ay4;

        return _m_amf;
    }


}  // End of namespace Grain
