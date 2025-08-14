//
//  SignalBandPassFilter.hpp
//
//  Created by Roald Christesen on 07.09.2017
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 22.07.2025
//

#ifndef SignalBandPassFilter_hpp
#define SignalBandPassFilter_hpp

#include "SignalFilter.hpp"


namespace Grain {

    class SignalBandPassFilter : public SignalFilter {
    public:
        enum class FilterType : int16_t {
            BandPass1 = 0
        };

    public:
        SignalBandPassFilter(int32_t sample_rate) noexcept;
        SignalBandPassFilter(int32_t sample_rate, FilterType filter_type, float low_freq, float high_freq) noexcept;
        ~SignalBandPassFilter() noexcept;

        const char *filterTypeName() const noexcept;

        void setFilterType(FilterType filter_type) noexcept;
        virtual void setFreqRange(float low_freq, float high_freq) noexcept override;

        virtual void reset() noexcept override;
        virtual float process(float input) noexcept override;

    protected:
        FilterType m_filter_type;
        double m_low_freq;
        double m_high_freq;

        double _m_a1, _m_b1, _m_b2;
        double _m_x1, _m_x2, _m_y1, _m_y2;

    };


} // End of namespace Grain

#endif // SignalBandPassFilter_hpp
