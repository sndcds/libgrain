#pragma once

#include <cstdint>

#if defined(__ARM_NEON)
    #include <arm_neon.h>
    #define GR_NEON 1
#elif defined(__AVX__)
    #include <immintrin.h>
    #define GR_AVX 1
#else
    #define GR_SCALAR 1
#endif

namespace Grain {

    struct float4 {
#if GR_NEON
        float32x4_t v;
#elif GR_AVX
        __m128 v;
#else
        float v[4];
#endif
    };

}