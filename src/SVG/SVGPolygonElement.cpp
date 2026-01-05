//
// SVGPolygonElement.cpp
//
// Created by Roald Christesen on 05.01.26
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGPolygonElement.hpp"


namespace Grain {

    void SVGPolygonElement::parseData(SVG* svg, const char* data) {

        SVGValuesParser values_parser(data);
        values_parser.setup(data);

        m_polygon = new class Polygon();

        double value;
        Vec2d pos;
        while (values_parser.next(value) > 0) {
            if ((values_parser.valueCount() % 2) != 0) {
                pos.x_ = value;
            }
            else {
                pos.y_ = value;
                m_polygon->addPoint(pos);
            }
        }
    }


    void SVGPolygonElement::fill(SVG* svg, GraphicContext& gc) noexcept {
        m_polygon->fill(gc);
    }


    void SVGPolygonElement::stroke(SVG* svg, GraphicContext& gc) noexcept {
        m_polygon->stroke(gc);
    }

} // End of namespace