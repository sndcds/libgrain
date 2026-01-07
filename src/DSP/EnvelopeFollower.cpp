//
//  EnvelopeFollower.cpp
//
//  Created by Roald Christesen on 22.05.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#include "DSP/EnvelopeFollower.hpp"
#include "Signal/Signal.hpp"
#include "Signal/SignalFilter.hpp"


namespace Grain {

	EnvelopeFollower::EnvelopeFollower(int32_t sample_rate, float attack_sec, float release_sec) noexcept
			: SignalFilter(sample_rate), attack_time_(attack_sec), release_time_(release_sec), envelope_(0.0f)
	{
		computeCoefficients();
	}

	void EnvelopeFollower::setAttackTime(float time_sec) noexcept {
		attack_time_ = std::max(time_sec, 0.0001f);
		computeCoefficients();
	}

	void EnvelopeFollower::setReleaseTime(float time_sec) noexcept {
		release_time_ = std::max(time_sec, 0.0001f);
		computeCoefficients();
	}

	void EnvelopeFollower::reset() noexcept {
		envelope_ = 0.0f;
	}

	void EnvelopeFollower::computeCoefficients() noexcept {
		attack_coef_ = 1.0f - std::exp(std::log(0.01f) / (attack_time_ * static_cast<float>(m_sample_rate)));
		release_coef_ = 1.0f - std::exp(std::log(0.01f) / (release_time_ * static_cast<float>(m_sample_rate)));
	}

	float EnvelopeFollower::process(float input) noexcept {
		float v = std::fabs(input);
		if (v > envelope_) {
			envelope_ += attack_coef_ * (v - envelope_);
		}
		else {
			envelope_ += release_coef_ * (v - envelope_);
		}

		return envelope_;
	}

	void EnvelopeFollower::process(float* input, float* output, int64_t length) noexcept {
		for (int64_t i = 0; i < length; i++) {
			output[i] = process(input[i]);
		}
	}

} // End of namespace