//
// SVGDefsElement.hpp
//
// Created by Roald Christesen on 03.01.2025
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGDefsElement_hpp
#define GrainSVGDefsElement_hpp

#include "SVG/SVGElement.hpp"


namespace Grain {

    class SVGDefsElement : public SVGElement {
    public:
        SVGDefsElement(SVGElement* parent) : SVGElement(parent) {
            m_type = ElementType::Defs;
        }

        ~SVGDefsElement() override {}

        void parse(SVG* svg, tinyxml2::XMLElement* xml_element) override;
    };
    
 } // End of namespace

#endif // GrainSVGDefsElement_hpp