//
// SVGElement.cpp
//
// Created by Roald Christesen on 27.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGElement.hpp"
#include "SVG/SVGPaintStyle.hpp"


namespace Grain {

    SVGElement::SVGElement(SVGElement* parent) : m_parent(parent) {
    }


    void SVGElement::initRootPaintStyle() noexcept {
        auto paint_style = mutablePaintStyle();
        if (paint_style != nullptr) {
            paint_style->m_attr_color.initWithFlags(SVGAttr::kFlag_IsRoot);
            paint_style->m_attr_color.setColor(RGBA(0.0f, 0.0f, 0.0f, 1.0f));

            paint_style->m_attr_fill.initWithFlags(SVGAttr::kFlag_IsRoot);
            paint_style->m_attr_stroke.initWithFlags(SVGAttr::kFlag_IsRoot);

            paint_style->m_attr_stroke_width.initWithFlags(SVGAttr::kFlag_IsRoot);
            paint_style->m_attr_stroke_width.setInt32(1, CSSUnit::Absolute);
        }
    }

} // End of namespace