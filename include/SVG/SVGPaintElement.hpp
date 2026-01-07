//
//  SVGPaintElement.hpp
//
//  Created by Roald Christesen on 12.01.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGPaintElement_hpp
#define GrainSVGPaintElement_hpp

#include "SVG/SVGElement.hpp"
#include "SVG/SVGPaintStyle.hpp"


namespace Grain {

    class GraphicContext;

    class SVGPaintElement : public SVGElement {
    protected:
        SVGPaintStyle paint_style_;
        Rectd bounds_;

    public:
        SVGPaintElement(SVGElement* parent) : SVGElement(parent) {
            paint_style_.setDefault();
            paint_style_.svg_element_ptr_ = this;
        }

        ~SVGPaintElement() {
        }

        bool canDraw() override { return true; }

        const SVGPaintStyle* paintStyle() const noexcept override { return &paint_style_; }
        SVGPaintStyle* mutablePaintStyle() noexcept override { return &paint_style_; }

        virtual void setCGStyle(GraphicContext& gc) const noexcept override {
            paint_style_.setGCSettings(gc);
        }

        bool doesFill() const noexcept { return valid_ == true && paint_style_.doesFill(); }
        bool doesStroke() const noexcept { return valid_ == true && paint_style_.doesStroke(); }

        void setPaintStyleByXMLElement(tinyxml2::XMLElement* xml_element) noexcept override {
            paint_style_.setByXMLElement(xml_element);
        }
    };

 } // End of namespace

#endif // GrainSVGPaintElement_hpp