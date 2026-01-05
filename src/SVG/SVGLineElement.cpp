//
// SVGLineElement.cpp
//
// Created by Roald Christesen on 30.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGLineElement.hpp"


namespace Grain {

    void SVGLineElement::stroke(SVG* svg, GraphicContext& gc) noexcept {
        if (m_valid) {
            gc.strokeLine(m_calc_p1, m_calc_p2);
        }
    }

} // End of namespace