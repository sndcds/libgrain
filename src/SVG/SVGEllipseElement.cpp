//
//  SVGEllipseElement.cpp
//
//  Created by Roald Christesen on 30.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGEllipseElement.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    void SVGEllipseElement::fill(SVG* svg, GraphicContext& gc) noexcept {
        if (valid_) {
            gc.fillEllipse(calc_center_, calc_rx_, calc_ry_);
        }
    }

    void SVGEllipseElement::stroke(SVG* svg, GraphicContext& gc) noexcept {
        if (valid_) {
            gc.strokeEllipse(calc_center_, calc_rx_, calc_ry_);
        }
    }

} // End of namespace