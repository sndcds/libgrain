//
//  SVGRectElement.cpp
//
//  Created by Roald Christesen on 30.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGRectElement.hpp"
#include "SVG/SVGGradient.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Color/Gradient.hpp"


namespace Grain {

    void SVGRectElement::draw(SVG* svg, GraphicContext& gc) noexcept {
    }


    // TODO: Rounded corners!
    void SVGRectElement::fill(SVG* svg, GraphicContext& gc) noexcept {
        if (doesFill()) {
            if (paint_style_.attr_fill_.set_command_ == SVGAttr::SetCommand::SetToURL) {
                paint_style_.attr_fill_.paint_server_ = svg->paintServerByID(paint_style_.attr_fill_.raw_);
                if (paint_style_.attr_fill_.paint_server_ != nullptr) {
                    auto gradient = (SVGGradient*)paint_style_.attr_fill_.paint_server_;
                    gc.clipRect(calc_rect_);
                    Vec2d start_pos = { calc_rect_.x_, calc_rect_.y_ };
                    Vec2d end_pos = { calc_rect_.x(), calc_rect_.y2() };
                    auto grain_gradient = gradient->gradientPtr();
                    grain_gradient->draw(&gc, start_pos, end_pos);
                    for (int32_t i = 0; i < grain_gradient->stopCount(); i++) {
                        auto stop = grain_gradient->mutStopPtrAtIndex(i);
                        RGBA color = stop->color(0);
                    }
                    gc.resetClip();
                }
                else {
                    // TODO: Error message!
                }
            }
            else {
                gc.fillRect(calc_rect_);
            }
        }
    }

    // TODO: Rounded corners!
    void SVGRectElement::stroke(SVG* svg, GraphicContext& gc) noexcept {
        if (doesStroke()) {
            gc.strokeRect(calc_rect_);
        }
    }

} // End of namespace