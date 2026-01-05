//
// SVGLineElement.hpp
//
// Created by Roald Christesen on 30.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGLineElement_hpp
#define GrainSVGLineElement_hpp

#include "SVG/SVGPaintElement.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    class SVGLineElement : public SVGPaintElement {

        friend class SVGGroupElement;

    protected:
        CSSValue m_x1;
        CSSValue m_y1;
        CSSValue m_x2;
        CSSValue m_y2;
        Vec2d m_calc_p1;
        Vec2d m_calc_p2;

    public:
        SVGLineElement(SVGElement* parent) : SVGPaintElement(parent) {
            m_type = ElementType::Line;
            m_x1.setInt32(0, CSSUnit::Absolute);
            m_y1.setInt32(0, CSSUnit::Absolute);
            m_x2.setInt32(0, CSSUnit::Absolute);
            m_y2.setInt32(0, CSSUnit::Absolute);
        }

        ~SVGLineElement() {
        }

        void setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept override {
            CSS::extractCSSValueFromStr(xml_element->Attribute("x1"), m_x1, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("y1"), m_y1, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("x2"), m_x2, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("y2"), m_y2, nullptr);
        }

        void validate() noexcept override {
            m_valid = true; // TODO: !
        }

        void stroke(SVG* svg, GraphicContext& gc) noexcept override;
    };
    
 } // End of namespace

#endif // GrainSVGLineElement_hpp