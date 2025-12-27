//
//  SignalIR.cpp
//
//  Created by Roald Christesen on 06.08.2022
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Signal/SignalIR.hpp"


namespace Grain {

    ErrorCode SignalIR::computeIR(
            Signal *sweep,
            Signal *recorded_sweep,
            int32_t recorded_sweep_channel,
            Signal* out_signal,
            int32_t out_signal_channel
    ) noexcept
    {
        auto err_code = ErrorCode::None;

        if (!sweep || !recorded_sweep || !out_signal) {
            return ErrorCode::NullPointer;
        }

        if (!sweep->hasData() || !recorded_sweep->hasData()) {
            return ErrorCode::NoData;
        }

        if (!recorded_sweep->hasChannel(recorded_sweep_channel) ||
            !out_signal->hasChannel(out_signal_channel)) {
            return ErrorCode::InvalidChannel;
        }

        int32_t sample_rate = sweep->sampleRate();

        if (recorded_sweep->sampleRate() != sample_rate ||
            out_signal->sampleRate() != sample_rate) {
            return ErrorCode::UnsupportedSampleRate;
        }

        int32_t convolve_partition_len = 16384;

        Signal* rs = nullptr;
        try {
            rs = recorded_sweep->createSignalFromChannel(recorded_sweep_channel, 0, -1);
            if (!rs) {
                Exception::throwSpecific(1, "Error creating Signal from channel");
            }
            rs->reverse();

            auto err = rs->convolveChannel(0, 0, -1, sweep, 0, 0, -1, out_signal, out_signal_channel, convolve_partition_len);
            Exception::throwStandard(err);

            out_signal->reverseChannel(out_signal_channel, 0, -1);
            out_signal->normalize(out_signal_channel, 0, out_signal->sampleCount(), 1.0f);
        }
        catch (const Exception& e) {
            err_code = e.code();
        }

        delete rs;

        return err_code;
    }

} // End of namespace Grain
