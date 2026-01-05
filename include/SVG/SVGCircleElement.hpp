//
// SVGCircleElement.hpp
//
// Created by Roald Christesen on 30.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGCircleElement_hpp
#define GrainSVGCircleElement_hpp

#include "SVG/SVGPaintElement.hpp"


namespace Grain {

    class SVGCircleElement : public SVGPaintElement {

        friend class SVGGroupElement;

    protected:
        CSSValue m_cx;
        CSSValue m_cy;
        CSSValue m_r;

        double m_calc_cx{};
        double m_calc_cy{};
        double m_calc_r{};

    public:
        SVGCircleElement(SVGElement* parent) : SVGPaintElement(parent) {
            m_type = ElementType::Circle;
            m_cx.setInt32(0, CSSUnit::Absolute);
            m_cy.setInt32(0, CSSUnit::Absolute);
            m_r.setInt32(10, CSSUnit::Absolute);
        }

        ~SVGCircleElement() {
        }

        void setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept override {
            CSS::extractCSSValueFromStr(xml_element->Attribute("cx"), m_cx, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("cy"), m_cy, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("r"), m_r, nullptr);

            m_calc_cx = m_cx.valueAsDouble();
            m_calc_cy = m_cy.valueAsDouble();
            m_calc_r = m_r.valueAsDouble();
        }

        void validate() noexcept override {
            m_valid = true; // Todo: !
        }

        void fill(SVG* svg, GraphicContext& gc) noexcept override;
        void stroke(SVG* svg, GraphicContext& gc) noexcept override;
    };

 } // End of namespace

#endif // GrainSVGCircleElement_hpp