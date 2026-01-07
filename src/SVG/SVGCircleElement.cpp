//
//  SVGCircleElement.cpp
//
//  Created by Roald Christesen on 30.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGCircleElement.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    void SVGCircleElement::fill(SVG* svg, GraphicContext& gc) noexcept {
        if (valid_) {
            gc.fillEllipse(calc_cx_, calc_cy_, calc_r_, calc_r_);
        }
    }

    void SVGCircleElement::stroke(SVG* svg, GraphicContext& gc) noexcept {
        if (valid_) {
            gc.strokeEllipse(calc_cx_, calc_cy_, calc_r_, calc_r_);
        }
    }

} // End of namespace