//
//  SVGEllipseElement.hpp
//
//  Created by Roald Christesen on 30.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGEllipseElement_hpp
#define GrainSVGEllipseElement_hpp

#include "SVG/SVGPaintElement.hpp"


namespace Grain {

    class SVGEllipseElement : public SVGPaintElement {

        friend class SVGGroupElement;

    protected:
        CSSValue cx_;
        CSSValue cy_;
        CSSValue rx_;
        CSSValue ry_;

        Vec2d calc_center_{};
        double calc_rx_{};
        double calc_ry_{};

    public:
        SVGEllipseElement(SVGElement* parent) : SVGPaintElement(parent) {
            type_ = ElementType::Ellipse;
            cx_.setInt32(0, CSSUnit::Absolute);
            cy_.setInt32(0, CSSUnit::Absolute);
            rx_.setInt32(10, CSSUnit::Absolute);
            ry_.setInt32(5, CSSUnit::Absolute);
        }

        ~SVGEllipseElement() {
        }

        void setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept override {
            CSS::extractCSSValueFromStr(xml_element->Attribute("cx"), cx_, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("cy"), cy_, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("rx"), rx_, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("ry"), ry_, nullptr);
        }

        void validate() noexcept override {
            valid_ = true; // TODO: !
        }

        void fill(SVG* svg, GraphicContext& gc) noexcept override;
        void stroke(SVG* svg, GraphicContext& gc) noexcept override;
    };

 } // End of namespace

#endif // GrainSVGEllipseElement_hpp