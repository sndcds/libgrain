//
//  SignalFormantFilter.hpp
//
//  Created by Roald Christesen on 21.04.2017
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainSignalFormantFilter_hpp
#define GrainSignalFormantFilter_hpp


#include "Signal/SignalButterworthFilter.hpp"
#include "Signal/SignalFilter.hpp"


namespace Grain {

class SignalFormantFilter : public SignalFilter {
public:
    static constexpr int32_t kMaxFormantCount = 5;

public:
    SignalFormantFilter(int32_t sample_rate) noexcept;
    virtual ~SignalFormantFilter() noexcept;

    [[nodiscard]] const char* className() const noexcept override { return "SignalFormantFilter"; }

    friend std::ostream& operator << (std::ostream& os, const SignalFormantFilter* o) {
        o == nullptr ? os << "SignalFormantFilter nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const SignalFormantFilter& o) {
        os << "formant_n: " << o.m_formant_n << std::endl;
        for (int32_t i = 0; i < o.m_formant_n; i++) {
            os << i << ": " << o.m_freq[i] << " Hz";
            os << " , lo: " << o.m_lo_freq[i] << " Hz";
            os << " , hi: " << o.m_freq[i] << " Hz";
            os << ", amp: " << o.m_amp[i];
            os << ", bandwidth: " << o.m_bw_cent[i] << " cent" << std::endl;
        }
        return os;
    }

    [[nodiscard]] int32_t formanCount() const noexcept { return m_formant_n; }
    void setFormantCount(int32_t count) noexcept { m_formant_n = std::clamp<int32_t>(count, 1, kMaxFormantCount); }

    ErrorCode setByData(int32_t formant_count, float* data) noexcept;

    void reset() noexcept override;
    float process(float input) noexcept override;


protected:
    int32_t m_formant_n = 3;               ///< Number of formants
    double m_freq[kMaxFormantCount];       ///< Center frequency, Hz
    double m_amp[kMaxFormantCount];        ///< Amplitudes
    double m_bw_cent[kMaxFormantCount];    ///< Bandwidth, cent
    double m_lo_freq[kMaxFormantCount];    ///< Low frequency
    double m_hi_freq[kMaxFormantCount];    ///< High frequency
    SignalButterworthFilter* m_filter[kMaxFormantCount]{};
};


} // End of namespace Grain

#endif // GrainSignalFormantFilter_hpp