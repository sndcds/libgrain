//
// SVGPaintElement.hpp
//
// Created by Roald Christesen on 12.01.2025
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGPaintElement_hpp
#define GrainSVGPaintElement_hpp

#include "SVG/SVGElement.hpp"
#include "SVG/SVGPaintStyle.hpp"


namespace Grain {

    class GraphicContext;

    class SVGPaintElement : public SVGElement {
    protected:
        SVGPaintStyle m_paint_style;
        Rectd m_bounds;

    public:
        SVGPaintElement(SVGElement* parent) : SVGElement(parent) {
            m_paint_style.setDefault();
            m_paint_style.m_svg_element_ptr = this;
        }

        ~SVGPaintElement() {
        }

        bool canDraw() override { return true; }

        const SVGPaintStyle* paintStyle() const noexcept override { return &m_paint_style; }
        SVGPaintStyle* mutablePaintStyle() noexcept override { return &m_paint_style; }

        virtual void setCGStyle(GraphicContext& gc) const noexcept override {
            m_paint_style.setGCSettings(gc);
        }

        bool doesFill() const noexcept { return m_valid == true && m_paint_style.doesFill(); }
        bool doesStroke() const noexcept { return m_valid == true && m_paint_style.doesStroke(); }

        void setPaintStyleByXMLElement(tinyxml2::XMLElement* xml_element) noexcept override {
            m_paint_style.setByXMLElement(xml_element);
        }
    };

 } // End of namespace

#endif // GrainSVGPaintElement_hpp