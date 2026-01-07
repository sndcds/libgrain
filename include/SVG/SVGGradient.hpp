//
//  SVGGradient.hpp
//
//  Created by Roald Christesen on 11.01.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGGradient_hpp
#define GrainSVGGradient_hpp

#include "Grain.hpp"
#include "SVG/SVG.hpp"
#include "SVG/SVGPaintServer.hpp"
#include "Color/RGBA.hpp"
#include "Color/Gradient.hpp"
#include "Type/List.hpp"


namespace Grain {

    class SVGGradientColorStop : Object {
        friend class SVGGradient;

    protected:
        CSSValue offset_;
        RGBA color_;

    public:
        SVGGradientColorStop() {
        }

        ~SVGGradientColorStop() {
        }

        void parse(SVG* svg, tinyxml2::XMLElement* xml_element);
        void setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept;
    };


    /**
     *  @class SVGGradient
     *  @brief Manages SVG gradient definitions and their properties.
     *
     *  This class provides functionality for creating and managing SVG gradients,
     *  including linear and radial gradients. It supports configuring color stops,
     *  gradient transformations, and color interpolation in different color spaces.
     */
    class SVGGradient : public SVGPaintServer {

    public:
        enum {
            kValueX1 = 0,
            kValueY1,
            kValueX2,
            kValueY2,

            kValueCX = 0,
            kValueCY,
            kValueR,
            kValueFX,
            kValueFY,

            kValueCount = 5
        };

    protected:
        SVGGradientType gradient_type_ = SVGGradientType::Linear;  ///< Type of the gradient (Linear or Radial)
        SVGGradientInterpolationMode color_interpolation_mode_ = SVGGradientInterpolationMode::sRGB;  ///< Color interpolation mode
        SVGGradientUnits units_ = SVGGradientUnits::ObjectBoundingBox;

        ObjectList<SVGGradientColorStop*>color_stops_;     ///< Gradient color stops
        double transform_{};                               ///< Gradient transformation matrix TODO: !!!!

        CSSValue values_[kValueCount];

        // TODO: Optional Attributes, href (or xlink:href in older SVGs): References another <linearGradient> to inherit properties.

        Gradient grain_gradient_;

    public:
        SVGGradient(SVGGradientType type, int32_t capacity = 16) noexcept;
        ~SVGGradient() noexcept;


        const char* className() const noexcept override { return "SVGGradient"; }

        friend std::ostream& operator << (std::ostream& os, const SVGGradient* o) {
            o == nullptr ? os << "SVGGradient nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const SVGGradient& o) {
            o.log(os, 0, "SVGGradient");
            return os;
        }

        void log(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const;


        Gradient* gradientPtr() noexcept { return &grain_gradient_; }

        void setColorInterpolation(SVGGradientInterpolationMode mode) noexcept;
        void setTransform(const char* transform) noexcept;
        void addColorStop(float offset, const RGBA& color) noexcept;
        void addColorStop(SVG* svg, tinyxml2::XMLElement* xml_element);

        void parse(SVG* svg, tinyxml2::XMLElement* xml_element);
        void setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept;

        ErrorCode toSVG(String& out_svg) const noexcept;  // TODO: !!!!!
    };


} // End of namespace

#endif // GrainSVGGradient_hpp
