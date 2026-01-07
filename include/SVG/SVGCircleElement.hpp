//
//  SVGCircleElement.hpp
//
//  Created by Roald Christesen on 30.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGCircleElement_hpp
#define GrainSVGCircleElement_hpp

#include "SVG/SVGPaintElement.hpp"


namespace Grain {

    class SVGCircleElement : public SVGPaintElement {

        friend class SVGGroupElement;

    protected:
        CSSValue cx_;
        CSSValue cy_;
        CSSValue r_;

        double calc_cx_{};
        double calc_cy_{};
        double calc_r_{};

    public:
        SVGCircleElement(SVGElement* parent) : SVGPaintElement(parent) {
            type_ = ElementType::Circle;
            cx_.setInt32(0, CSSUnit::Absolute);
            cy_.setInt32(0, CSSUnit::Absolute);
            r_.setInt32(10, CSSUnit::Absolute);
        }

        ~SVGCircleElement() {
        }

        void setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept override {
            CSS::extractCSSValueFromStr(xml_element->Attribute("cx"), cx_, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("cy"), cy_, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("r"), r_, nullptr);

            calc_cx_ = cx_.valueAsDouble();
            calc_cy_ = cy_.valueAsDouble();
            calc_r_ = r_.valueAsDouble();
        }

        void validate() noexcept override {
            valid_ = true; // Todo: !
        }

        void fill(SVG* svg, GraphicContext& gc) noexcept override;
        void stroke(SVG* svg, GraphicContext& gc) noexcept override;
    };

 } // End of namespace

#endif // GrainSVGCircleElement_hpp