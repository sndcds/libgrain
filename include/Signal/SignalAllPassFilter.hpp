//
//  SignalAllPassFilter.hpp
//
//  Created by Roald Christesen on 16.09.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 23.07.2025
//

/**
 *  @brief A feedback allpass filter with variable feedback amount and up to 10 stages.
 *
 *  The allpass filter is a signal processing filter that maintains equal gain for all frequencies but alters the phase relationship among them. Unlike most types of filters that attenuate certain frequencies, the allpass filter allows all frequencies to pass through without changing their amplitude.
 *
 *  This implementation supports up to 10 stages, and the feedback amount can be adjusted dynamically to control the filter's characteristics.
 */

#ifndef SignalAllPassFilter_hpp
#define SignalAllPassFilter_hpp

#include "Signal/SignalFilter.hpp"


namespace Grain {


    class SignalAllPassFilter : public SignalFilter {
    public:
        static constexpr int32_t kMaxStageCount = 10;

    public:
        explicit SignalAllPassFilter(int32_t sample_rate) noexcept;
        ~SignalAllPassFilter() noexcept override;

        [[nodiscard]] float freq() const noexcept override { return m_freq; }
        [[nodiscard]] float feedback() const noexcept override { return m_feedback_amount; }
        [[nodiscard]] int32_t stageCount() const noexcept override { return m_stage_count; }
        [[nodiscard]] bool feedbackPhaseInverted() const noexcept { return m_feedback_phase_inverted; }

        void setFreq(float freq) noexcept override;
        void setFeedback(float amount, bool phase_inverted) noexcept override;
        void setStageCount(int32_t stage_count) noexcept override;

        void reset() noexcept override;
        float process(float input) noexcept override;

    protected:
        double m_freq;                  ///< Filter frequence in Hz
        double m_feedback_amount;       ///< Feedback amount in range 0 to 1
        bool m_feedback_phase_inverted; ///< If set to true, the phase will be inverted
        int32_t m_stage_count;          ///< Number of feedback stages

        double _m_coef{};
        double _m_feedback_gain{};
        bool _m_feedback_enabled = false;

        double _m_lx[kMaxStageCount]{};
        double _m_ly[kMaxStageCount]{};
    };


} // End of namespace Grain

#endif // SignalAllPassFilter_hpp
