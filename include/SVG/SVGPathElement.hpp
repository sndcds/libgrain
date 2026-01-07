//
//  SVGPathElement.hpp
//
//  Created by Roald Christesen on 30.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGPathElement_hpp
#define GrainSVGPathElement_hpp

#include "SVG/SVGPaintElement.hpp"


namespace Grain {

    class SVGPathElement : public SVGPaintElement {

    protected:
        GraphicCompoundPath* compound_path_ = nullptr;

    public:
        SVGPathElement(SVGElement *parent) : SVGPaintElement(parent) {
            type_ = ElementType::Path;
        }

        ~SVGPathElement();

        void parsePathData(SVG* svg, const char* data);

        void validate() noexcept override {
            valid_ = true; // Todo:
        }

        void fill(SVG* svg, GraphicContext& gc) noexcept override;
        void stroke(SVG* svg, GraphicContext& gc) noexcept override;
    };

} // End of namespace

#endif // GrainSVGPathElement_hpp
