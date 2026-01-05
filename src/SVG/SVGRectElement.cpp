//
// SVGRectElement.cpp
//
// Created by Roald Christesen on 30.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGRectElement.hpp"
#include "SVG/SVGGradient.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Color/Gradient.hpp"


namespace Grain {

    void SVGRectElement::draw(SVG* svg, GraphicContext& gc) noexcept {
    }


    // Todo: Rounded corners!
    void SVGRectElement::fill(SVG* svg, GraphicContext& gc) noexcept {
        if (doesFill()) {
            if (m_paint_style.m_attr_fill.m_set_command == SVGAttr::SetCommand::SetToURL) {
                m_paint_style.m_attr_fill.m_paint_server = svg->paintServerByID(m_paint_style.m_attr_fill.m_raw);
                if (m_paint_style.m_attr_fill.m_paint_server != nullptr) {
                    auto gradient = (SVGGradient*)m_paint_style.m_attr_fill.m_paint_server;
                    gc.clipRect(m_calc_rect);
                    Vec2d start_pos = { m_calc_rect.x_, m_calc_rect.y_ };
                    Vec2d end_pos = { m_calc_rect.x(), m_calc_rect.y2() };
                    auto grain_gradient = gradient->gradientPtr();
                    grain_gradient->draw(&gc, start_pos, end_pos);
                    for (int32_t i = 0; i < grain_gradient->stopCount(); i++) {
                        auto stop = grain_gradient->mutStopPtrAtIndex(i);
                        RGBA color = stop->color(0);
                    }
                    gc.resetClip();
                }
                else {
                    // Todo: Error message!
                }
            }
            else {
                gc.fillRect(m_calc_rect);
            }
        }
    }

    // Todo: Rounded corners!
    void SVGRectElement::stroke(SVG* svg, GraphicContext& gc) noexcept {
        if (doesStroke()) {
            gc.strokeRect(m_calc_rect);
        }
    }

} // End of namespace