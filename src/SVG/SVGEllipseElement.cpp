//
// SVGEllipseElement.cpp
//
// Created by Roald Christesen on 30.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGEllipseElement.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    void SVGEllipseElement::fill(SVG* svg, GraphicContext& gc) noexcept {
        if (m_valid) {
            gc.fillEllipse(m_calc_center, m_calc_rx, m_calc_ry);
        }
    }

    void SVGEllipseElement::stroke(SVG* svg, GraphicContext& gc) noexcept {
        if (m_valid) {
            gc.strokeEllipse(m_calc_center, m_calc_rx, m_calc_ry);
        }
    }

} // End of namespace