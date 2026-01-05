//
// SVGPolygonElement.hpp
//
// Created by Roald Christesen on 05.01.26
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGPolygonElement_hpp
#define GrainSVGPolygonElement_hpp

#include "SVG/SVGPaintElement.hpp"


namespace Grain {

    class SVGPolygonElement : public SVGPaintElement {
    protected:
        class Polygon* m_polygon = nullptr;
        bool m_closed = false;

    public:
        SVGPolygonElement(SVGElement* parent) : SVGPaintElement(parent) {
            m_type = ElementType::Path;
        }

        ~SVGPolygonElement() {
            // Todo: Delete m_polygon;
        }

        void close() noexcept { m_closed = true; }
        bool isClosed() const noexcept { return m_closed; }

        void parseData(SVG* svg, const char* data);

        void validate() noexcept override { m_valid = true; }

        void fill(SVG* svg, GraphicContext& gc) noexcept override;
        void stroke(SVG* svg, GraphicContext& gc) noexcept override;
    };

} // End of namespace

#endif // GrainSVGPolygonElement_hpp
