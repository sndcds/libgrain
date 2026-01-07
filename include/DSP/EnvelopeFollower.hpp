//
//  EnvelopeFollower.hpp
//
//  Created by Roald Christesen on 22.05.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainEnvelopeFollower_hpp
#define GrainEnvelopeFollower_hpp

#include "Type/Object.hpp"
#include "Signal/SignalFilter.hpp"


namespace Grain {

    class Signal;


    class EnvelopeFollower : public SignalFilter {
    public:
        explicit EnvelopeFollower(int32_t sample_rate, float attack_sec = 0.005f, float release_sec = 0.05f) noexcept;

        [[nodiscard]] const char* className() const noexcept override { return "EnvelopeFollower"; }

        void setAttackTime(float time_sec) noexcept;
        void setReleaseTime(float time_sec) noexcept;
        void reset() noexcept override;

        [[nodiscard]] float process(float input) noexcept override;

        // Optional: process an array of samples
        void process(float* input, float* output, int64_t length) noexcept;

        [[nodiscard]] float current() const noexcept { return envelope_; }

    private:
        float attack_time_;      ///< seconds
        float release_time_;     ///< seconds
        float attack_coef_{};    ///< computed from attack_time
        float release_coef_{};   ///< computed from release_time
        float envelope_;         ///< current envelope value

        void computeCoefficients() noexcept;
    };


} // End of namespace

#endif // GrainEnvelopeFollower_hpp
