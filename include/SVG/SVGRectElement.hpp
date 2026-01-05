//
// SVGRectElement.hpp
//
// Created by Roald Christesen on 30.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGRectElement_hpp
#define GrainSVGRectElement_hpp

#include "SVG/SVGPaintElement.hpp"


namespace Grain {

    class GraphicContext;

    class SVGRectElement : public SVGPaintElement {
        friend class SVGGroupElement;

    protected:
        CSSValue m_x;
        CSSValue m_y;
        CSSValue m_width;
        CSSValue m_height;
        CSSValue m_rx;
        CSSValue m_ry;

        Rectd m_calc_rect{};        ///< Calculated rect
        double m_calc_rx{};
        double m_calc_ry{};

    public:
        SVGRectElement(SVGElement* parent) : SVGPaintElement(parent) {
            m_type = ElementType::Rect;
            m_x.setInt32(0, CSSUnit::Absolute);
            m_y.setInt32(0, CSSUnit::Absolute);
            m_width.setInt32(0, CSSUnit::Absolute);
            m_height.setInt32(0, CSSUnit::Absolute);
            m_rx.setInt32(0, CSSUnit::Absolute);
            m_ry.setInt32(0, CSSUnit::Absolute);
        }

        ~SVGRectElement() override = default;

        void setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept override {
            CSS::extractCSSValueFromStr(xml_element->Attribute("x"), m_x, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("y"), m_y, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("width"), m_width, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("height"), m_height, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("rx"), m_rx, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("ry"), m_ry, nullptr);

            m_calc_rect.set(m_x.valueSVGPixel(), m_y.valueSVGPixel(), m_width.valueSVGPixel(), m_height.valueSVGPixel());
        }

        void validate() noexcept override {
            m_valid = true;
        }

        void draw(SVG* svg, GraphicContext& gc) noexcept override;
        void fill(SVG* svg, GraphicContext& gc) noexcept override;
        void stroke(SVG* svg, GraphicContext& gc) noexcept override;
    };

 } // End of namespace

#endif // GrainSVGRectElement_hpp