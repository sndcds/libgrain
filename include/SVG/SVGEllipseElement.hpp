//
// SVGEllipseElement.hpp
//
// Created by Roald Christesen on 30.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGEllipseElement_hpp
#define GrainSVGEllipseElement_hpp

#include "SVG/SVGPaintElement.hpp"


namespace Grain {

    class SVGEllipseElement : public SVGPaintElement {

        friend class SVGGroupElement;

    protected:
        CSSValue m_cx;
        CSSValue m_cy;
        CSSValue m_rx;
        CSSValue m_ry;

        Vec2d m_calc_center{};
        double m_calc_rx{};
        double m_calc_ry{};

    public:
        SVGEllipseElement(SVGElement* parent) : SVGPaintElement(parent) {
            m_type = ElementType::Ellipse;
            m_cx.setInt32(0, CSSUnit::Absolute);
            m_cy.setInt32(0, CSSUnit::Absolute);
            m_rx.setInt32(10, CSSUnit::Absolute);
            m_ry.setInt32(5, CSSUnit::Absolute);
        }

        ~SVGEllipseElement() {
        }

        void setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept override {
            CSS::extractCSSValueFromStr(xml_element->Attribute("cx"), m_cx, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("cy"), m_cy, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("rx"), m_rx, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("ry"), m_ry, nullptr);
        }

        void validate() noexcept override {
            m_valid = true; // TODO: !
        }

        void fill(SVG* svg, GraphicContext& gc) noexcept override;
        void stroke(SVG* svg, GraphicContext& gc) noexcept override;
    };

 } // End of namespace

#endif // GrainSVGEllipseElement_hpp