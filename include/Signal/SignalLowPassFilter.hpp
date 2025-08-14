//
//  SignalLowPassFilter.hpp
//
//  Created by Roald Christesen on 09.09.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 22.07.2025
//

#ifndef SignalLowPassFilter_hpp
#define SignalLowPassFilter_hpp

#include "SignalFilter.hpp"


namespace Grain {

    class SignalLowPassFilter : public SignalFilter {
    public:
        explicit SignalLowPassFilter(int32_t sampleRate) noexcept;
        ~SignalLowPassFilter() noexcept override = default;

        void setFreq(float freq) noexcept override { setFreq(freq, static_cast<float>(m_resonance)); }
        void setFreq(float freq, float resonance) noexcept override;

        void reset() noexcept override;
        float process(float input) noexcept override;

    protected:
        double m_freq = 8000.0;	       ///< Cutoff frequency in Hz
        double m_resonance = 0.0;      ///< Resonance, 0-1
        double _m_q{};
        double _m_stage[4]{};
        double _m_delay[4]{};
        double _m_p{};
        double _m_k{};
    };


} // End of namespace Grain

#endif // SignalLowPassFilter_hpp
