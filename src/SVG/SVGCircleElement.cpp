//
// SVGCircleElement.cpp
//
// Created by Roald Christesen on 30.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGCircleElement.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    void SVGCircleElement::fill(SVG* svg, GraphicContext& gc) noexcept {
        if (m_valid) {
            gc.fillEllipse(m_calc_cx, m_calc_cy, m_calc_r, m_calc_r);
        }
    }

    void SVGCircleElement::stroke(SVG* svg, GraphicContext& gc) noexcept {
        if (m_valid) {
            gc.strokeEllipse(m_calc_cx, m_calc_cy, m_calc_r, m_calc_r);
        }
    }

} // End of namespace