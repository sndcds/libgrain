//
//  DSP.hpp
//
//  Created by Roald Christesen on 25.01.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//


#ifndef GrainDSP_hpp
#define GrainDSP_hpp

#include "Grain.hpp"


namespace Grain {

    class DSP {
    public:

        /**
         *  @brief Window types, used for filtering, FFT etc.
         */
        enum class WindowType {
            Kaiser = 0,
            Sinc,       // Lanczos
            Sine,       // Hanning if Beta = 2
            Hanning,
            Hamming,
            Blackman,
            FlatTop,
            BlackmanHarris,
            BlackmanNuttall,
            Nuttall,
            KaiserBessel,
            Trapezoid,
            Gauss,
            First = 0,
            Last = Gauss,
        };

        static float hanningWindow(float t) noexcept;
        static float hanningLeft(float t) noexcept;
        static bool hanningWindowSymmetric(int32_t window_size, float* out_data) noexcept;
        static bool hanningWindowPeriodic(int32_t window_size, float* out_data) noexcept;
        static ErrorCode window(int32_t window_size, WindowType window_type, float alpha, float beta, bool unity_gain, float* out_data) noexcept;
    };


} // End of namespace Grain

#endif // GrainDSP_hpp
