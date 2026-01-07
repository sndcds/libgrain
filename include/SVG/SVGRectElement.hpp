//
//  SVGRectElement.hpp
//
//  Created by Roald Christesen on 30.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGRectElement_hpp
#define GrainSVGRectElement_hpp

#include "SVG/SVGPaintElement.hpp"


namespace Grain {

    class GraphicContext;

    class SVGRectElement : public SVGPaintElement {
        friend class SVGGroupElement;

    protected:
        CSSValue x_;
        CSSValue y_;
        CSSValue width_;
        CSSValue height_;
        CSSValue rx_;
        CSSValue ry_;

        Rectd calc_rect_{};        ///< Calculated rect
        double calc_rx_{};
        double calc_ry_{};

    public:
        SVGRectElement(SVGElement* parent) : SVGPaintElement(parent) {
            type_ = ElementType::Rect;
            x_.setInt32(0, CSSUnit::Absolute);
            y_.setInt32(0, CSSUnit::Absolute);
            width_.setInt32(0, CSSUnit::Absolute);
            height_.setInt32(0, CSSUnit::Absolute);
            rx_.setInt32(0, CSSUnit::Absolute);
            ry_.setInt32(0, CSSUnit::Absolute);
        }

        ~SVGRectElement() override = default;

        void setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept override {
            CSS::extractCSSValueFromStr(xml_element->Attribute("x"), x_, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("y"), y_, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("width"), width_, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("height"), height_, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("rx"), rx_, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("ry"), ry_, nullptr);

            calc_rect_.set(x_.valueSVGPixel(), y_.valueSVGPixel(), width_.valueSVGPixel(), height_.valueSVGPixel());
        }

        void validate() noexcept override {
            valid_ = true;
        }

        void draw(SVG* svg, GraphicContext& gc) noexcept override;
        void fill(SVG* svg, GraphicContext& gc) noexcept override;
        void stroke(SVG* svg, GraphicContext& gc) noexcept override;
    };

 } // End of namespace

#endif // GrainSVGRectElement_hpp