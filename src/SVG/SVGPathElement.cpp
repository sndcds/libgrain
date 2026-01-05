//
// SVGPathElement.cpp
//
// Created by Roald Christesen on 30.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGPathElement.hpp"
#include "SVG/SVG.hpp"
#include "Graphic/GraphicContext.hpp"
#include "2d/GraphicCompoundPath.hpp"


namespace Grain {

    SVGPathElement::~SVGPathElement() {

        delete m_compound_path;
    }

    void SVGPathElement::parsePathData(SVG* svg, const char* data) {
        m_compound_path = new(std::nothrow) GraphicCompoundPath();
        if (!m_compound_path) {
            Exception::throwStandard(ErrorCode::MemCantAllocate);
        }

        auto path_parser = new(std::nothrow) SVGPathParser(svg, m_compound_path);
        if (!path_parser) {
            Exception::throwStandard(ErrorCode::MemCantAllocate);
        }
        else {
            path_parser->parsePathData(data);
            delete path_parser;
        }
    }

    void SVGPathElement::fill(SVG* svg, GraphicContext& gc) noexcept {
        if (m_compound_path) {
            m_compound_path->fill(&gc);
        }
    }


    void SVGPathElement::stroke(SVG* svg, GraphicContext& gc) noexcept {
        if (m_compound_path) {
            m_compound_path->stroke(&gc);
        }
    }

} // End of namespace