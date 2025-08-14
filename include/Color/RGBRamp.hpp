//
//  RGBRamp.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainRGBRamp_hpp
#define GrainRGBRamp_hpp

#include "Color/RGB.hpp"


namespace Grain {


    /**
     *  @brief Fast and easy RGB color ramp with equally distributed color spots.
     *
     *  It can be looked up in different modes.
     */
    class RGBRamp {

        enum {
            kMaxSpotCount = 17
        };

        int32_t m_spot_count = 0;
        RGB m_spots_colors[kMaxSpotCount];

    public:
        RGBRamp();

        void dedaultLocationRamp() noexcept;

        int32_t spotCount() const noexcept { return m_spot_count; }
        bool spotColorAtIndex(int32_t index, RGB& out_color) const noexcept;

        void addSpot(const RGB& color) noexcept;
        void setSpotCount(int32_t spot_count) noexcept { m_spot_count = spot_count; }
        void setSpotColorAtIndex(int32_t index, const RGB& color) noexcept;

        void lookup(float t, RGB& out_color) const noexcept;
        void lookup(float t, float* out_values) const noexcept;
        void lookupRing(float t, RGB& out_color) const noexcept;
        void lookupRing(float t, float* out_values) const noexcept;
    };


} // End of namespace Grain

#endif // GrainRGBRamp_hpp
