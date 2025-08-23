//
//  StrokeStyle.cpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 22.08.2025
//

#include "Graphic/StrokeStyle.hpp"
// #include "2d/GraphicPath.hpp"


namespace Grain {

    StrokeStyle::StrokeStyle(uint32_t count, const float* pattern, float phase, float width) noexcept {
        m_count = pattern == nullptr ? 2 : count;
        m_pattern = (float*)std::malloc(m_count * sizeof(float));
        m_phase = phase;
        m_width = width;
        m_capStyle = StrokeCapStyle::Butt;
        m_joinStyle = StrokeJoinStyle::Miter;

        if (m_pattern) {
            if (pattern) {
                for (uint32_t i = 0; i < count; i++) {
                    m_pattern[i] = pattern[i];
                }
            }
            else {
                for (uint32_t i = 0; i < count; i++) {
                    m_pattern[i] = 1.0f;
                }
            }
        }
    }


    StrokeStyle::~StrokeStyle() noexcept {
        std::free(m_pattern);
    }

} // End of namespace Grain.
