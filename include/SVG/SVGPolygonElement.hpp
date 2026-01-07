//
//  SVGPolygonElement.hpp
//
//  Created by Roald Christesen on 05.01.2026
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGPolygonElement_hpp
#define GrainSVGPolygonElement_hpp

#include "SVG/SVGPaintElement.hpp"


namespace Grain {

    class SVGPolygonElement : public SVGPaintElement {
    protected:
        class Polygon* polygon_ = nullptr;
        bool closed_ = false;

    public:
        SVGPolygonElement(SVGElement* parent) : SVGPaintElement(parent) {
            type_ = ElementType::Path;
        }

        ~SVGPolygonElement() {
            // Todo: Delete m_polygon;
        }

        void close() noexcept { closed_ = true; }
        bool isClosed() const noexcept { return closed_; }

        void parseData(SVG* svg, const char* data);

        void validate() noexcept override { valid_ = true; }

        void fill(SVG* svg, GraphicContext& gc) noexcept override;
        void stroke(SVG* svg, GraphicContext& gc) noexcept override;
    };

} // End of namespace

#endif // GrainSVGPolygonElement_hpp
