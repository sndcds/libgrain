//
//  SVGRootElement.hpp
//
//  Created by Roald Christesen on 03.01.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGRootElement_hpp
#define GrainSVGRootElement_hpp

#include "SVG/SVGGroupElement.hpp"
#include "CSS/CSS.hpp"


namespace Grain {

    class SVG;
    class SVGElement;

    class SVGRootElement : public SVGGroupElement {
    protected:
        String version_;

        CSSValue x_;
        CSSValue y_;
        CSSValue width_;
        CSSValue height_;
        CSSValue viewport_x_;
        CSSValue viewport_y_;
        CSSValue viewport_width_;
        CSSValue viewport_height_;

        double m_pixel_size = 2.54 / 96;

    public:
        SVGRootElement(SVGElement* parent) : SVGGroupElement(parent) {
            type_ = ElementType::SVGRoot;
        }

        ~SVGRootElement() = default;


        friend std::ostream& operator << (std::ostream& os, const SVGRootElement* o) {
            o == nullptr ? os << "SVGRootElement nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const SVGRootElement& o) {
            o.log(os, 0, "SVGRootElement");
            return os;
        }

        void log(std::ostream& os, int32_t indent, const char* label) const;


        void setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept;
        void setViewBox(const char* str) noexcept;

        void parse(SVG* svg, tinyxml2::XMLElement* xml_element);

        void validate() noexcept {
            valid_ = true; // Todo: !
        }
    };

} // End of namespace

#endif // GrainSVGRootElement_hpp