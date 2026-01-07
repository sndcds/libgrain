//
//  SVGLineElement.hpp
//
//  Created by Roald Christesen on 30.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGLineElement_hpp
#define GrainSVGLineElement_hpp

#include "SVG/SVGPaintElement.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    class SVGLineElement : public SVGPaintElement {

        friend class SVGGroupElement;

    protected:
        CSSValue x1_;
        CSSValue y1_;
        CSSValue x2_;
        CSSValue y2_;
        Vec2d calc_p1_;
        Vec2d calc_p2_;

    public:
        SVGLineElement(SVGElement* parent) : SVGPaintElement(parent) {
            type_ = ElementType::Line;
            x1_.setInt32(0, CSSUnit::Absolute);
            y1_.setInt32(0, CSSUnit::Absolute);
            x2_.setInt32(0, CSSUnit::Absolute);
            y2_.setInt32(0, CSSUnit::Absolute);
        }

        ~SVGLineElement() {
        }

        void setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept override {
            CSS::extractCSSValueFromStr(xml_element->Attribute("x1"), x1_, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("y1"), y1_, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("x2"), x2_, nullptr);
            CSS::extractCSSValueFromStr(xml_element->Attribute("y2"), y2_, nullptr);
        }

        void validate() noexcept override {
            valid_ = true; // TODO: !
        }

        void stroke(SVG* svg, GraphicContext& gc) noexcept override;
    };
    
 } // End of namespace

#endif // GrainSVGLineElement_hpp