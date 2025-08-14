//
//  WeightedSamples.hpp
//
//  Created by Roald Christesen on 19.03.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

#ifndef GrainWeightedSamples_hpp
#define GrainWeightedSamples_hpp

#include "Grain.hpp"


namespace Grain {

    class Bezier;


    class WeightedSamples {
    public:
        enum {
            MIN_RESOLUTION = 2,
            MAX_RESOLUTION = std::numeric_limits<int32_t>::max()
        };

        enum {
            ERR_RESOLUTION_NOT_ALLOWED = 0
        };

    private:
        int32_t m_resolution = 0;
        int32_t m_max_resolution = 0;
        float *m_samples = nullptr;
        float *m_weights = nullptr;
        bool m_use_extern_mem = false;

        ErrorCode m_last_err = ErrorCode::None;


    public:
        WeightedSamples(int32_t resolution) noexcept;
        WeightedSamples(int32_t resolution, float *mem) noexcept;
        ~WeightedSamples() noexcept;


        int32_t resolution() const noexcept { return m_resolution; }
        float *mutSamplesPtr() const noexcept { return m_samples; }
        const float *samplesPtr() const noexcept { return m_samples; }
        const float *weightsPtr() const noexcept { return m_weights; }
        float sampleAtIndex(int32_t index) const noexcept;
        float weightAtIndex(int32_t index) const noexcept;
        void minMaxY(float *out_min_y, float *out_max_y) const noexcept;

        void clear() noexcept;
        ErrorCode setResolution(int32_t resolution) noexcept;
        void setSample(int32_t index, float value) noexcept;
        void setWeight(int32_t index, float weight) noexcept;
        void addSample(float value, int32_t index, int32_t resolution) noexcept;
        void addWeightedSample(int32_t index, float value, float weight) noexcept;
        void addBezier(const Bezier &bezier, int32_t bezier_resolution) noexcept;

        void finish() noexcept;
        float lookup(float t) const noexcept;
    };


} // End of namespace Grain

#endif // GrainWeightedSamples_hpp
