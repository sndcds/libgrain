//
//  SVGPathElement.cpp
//
//  Created by Roald Christesen on 30.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGPathElement.hpp"
#include "SVG/SVG.hpp"
#include "Graphic/GraphicContext.hpp"
#include "2d/GraphicCompoundPath.hpp"


namespace Grain {

    SVGPathElement::~SVGPathElement() {

        delete compound_path_;
    }

    void SVGPathElement::parsePathData(SVG* svg, const char* data) {
        compound_path_ = new(std::nothrow) GraphicCompoundPath();
        if (!compound_path_) {
            Exception::throwStandard(ErrorCode::MemCantAllocate);
        }

        auto path_parser = new(std::nothrow) SVGPathParser(svg, compound_path_);
        if (!path_parser) {
            Exception::throwStandard(ErrorCode::MemCantAllocate);
        }
        else {
            path_parser->parsePathData(data);
            delete path_parser;
        }
    }

    void SVGPathElement::fill(SVG* svg, GraphicContext& gc) noexcept {
        if (compound_path_) {
            compound_path_->fill(&gc);
        }
    }


    void SVGPathElement::stroke(SVG* svg, GraphicContext& gc) noexcept {
        if (compound_path_) {
            compound_path_->stroke(&gc);
        }
    }

} // End of namespace