//
//  SignalIR.hpp
//
//  Created by Roald Christesen on 06.08.2022
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainSignalIR_hpp
#define GrainSignalIR_hpp

#include "Grain.hpp"
#include "Signal.hpp"


namespace Grain {

    class SignalIR {
    public:
        static ErrorCode computeIR(
                Signal *sweep,
                Signal *recorded_sweep,
                int32_t recorded_sweep_channel,
                Signal* out_signal,
                int32_t out_signal_channel
        ) noexcept;
    };

} // End of namespace Grain

#endif // GrainSignalIR_hpp