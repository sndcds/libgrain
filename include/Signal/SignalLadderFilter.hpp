//
//  SignalLadderFilter.hpp
//
//  Created by Roald Christesen on 30.09.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 22.07.2025
//

#ifndef SignalLadderFilter_hpp
#define SignalLadderFilter_hpp

#include "Signal/SignalFilter.hpp"


namespace Grain {

    class SignalLadderFilter : public SignalFilter {
    public:
        explicit SignalLadderFilter(int32_t sample_rate) noexcept;
        ~SignalLadderFilter() noexcept override = default;

        void setFreq(float freq) noexcept override { setFreq(freq, 0.0f); }
        void setFreq(float freq, float resonance) noexcept override;

        void reset() noexcept override;
        float process(float input) noexcept override;

    protected:
        double m_freq = 8000.0;     ///< Cutoff frequency in Hz
        double m_resonance = 0.0;   ///< Resonance, 0-1
        double m_drive{};
        double _m_ipi{};
        double _m_kfc{};
        double _m_kf{};
        double _m_kfcr{};
        double _m_kacr{};
        double _m_k2vg{};
        double _m_az1{};            ///< Filter delays
        double _m_az2 {};
        double _m_az3{};
        double _m_az4{};
        double _m_az5{};
        double _m_ay1{};
        double _m_ay2{};
        double _m_ay3{};
        double _m_ay4{};
        double _m_amf{};
        double _m_i2v = 40000.0;    ///< Twice the 'thermal voltage of a transistor'
    };


} // End of namespace Grain

#endif // SignalLadderFilter_hpp
