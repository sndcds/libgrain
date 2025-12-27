//
//  StrokeStyle.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 22.08.2025
//

#ifndef GrainStrokeStyle_hpp
#define GrainStrokeStyle_hpp

#include "Grain.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    class StrokeStyle {
    public:
        StrokeStyle(uint32_t count, const float* pattern, float phase, float width) noexcept;
        ~StrokeStyle() noexcept;

        [[nodiscard]] uint32_t count() const noexcept { return count_; }
        [[nodiscard]] float* patternPtr() const noexcept { return m_pattern; }
        [[nodiscard]] float phase() const noexcept { return m_phase; }
        [[nodiscard]] float width() const noexcept { return width_; }

        void setWidth(float width) noexcept { width_ = width; }
        void setCapStyle(StrokeCapStyle cap_style) noexcept { m_capStyle = cap_style; }
        void setJoinStyle(StrokeJoinStyle join_style) noexcept { m_joinStyle = join_style; }

    protected:
        uint32_t count_;
        float* m_pattern;
        float m_phase;
        float width_;
        StrokeCapStyle m_capStyle;
        StrokeJoinStyle m_joinStyle;
    };


} // End of namespace Grain

#endif // GrainStrokeStyle_hpp
