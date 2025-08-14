//
//  SignalButterworthFilter.hpp
//
//  Created by Roald Christesen on 09.06.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 22.07.2025
//

//
//  Second order low pass filter
//  Second order high pass filter
//  Fourth order band pass filter
//  Fourth order band stop filter
//
//  Algorithms taken from http://www.exstrom.com/journal/sigproc/
//

#ifndef SignalButterworthFilter_hpp
#define SignalButterworthFilter_hpp

#include "SignalFilter.hpp"

namespace Grain {

    class SignalButterworthFilter : public SignalFilter {
    public:
        enum class FilterType : int16_t {
            LowPass = 0,    ///< 2nd order low pass
            HighPass,		///< 2nd order high pass
            BandPass,		///< 4th order band pass
            BandStop		///< 4th order band stop
        };

    public:
        SignalButterworthFilter(int32_t sample_rate) noexcept;
        SignalButterworthFilter(int32_t sample_rate, FilterType filter_type, float freq) noexcept;
        ~SignalButterworthFilter() noexcept;


        const char *filterTypeName() const noexcept;

        void setFilterType(FilterType filterType) noexcept;
        virtual void setFreq(float freq) noexcept override;
        virtual void setFreq(float freq, float resonance) noexcept override { setFreq(freq); }
        virtual void setFreqRange(float low_freq, float high_freq) noexcept override;

        virtual void reset() noexcept override;
        virtual float process(float input) noexcept override;

    protected:
        FilterType m_filter_type;
        double m_freq;       ///< Cutoff frequency in Hz used in low pass and high pass
        double m_low_freq;   ///< Low cutoff frequency in Hz used in band pass and band stop
        double m_high_freq;  ///< High cutoff frequency in Hz used in band pass and band stop

        double m_r{};
        double m_s{};
        double m_a{};
        double m_d1{};
        double m_d2{};
        double m_d3{};
        double m_d4{};
        double m_w[5]{};
    };


} // End of namespace Grain

#endif // SignalButterworthFilter_hpp
