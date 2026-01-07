//
//  SVGElement.hpp
//
//  Created by Roald Christesen on 27.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGElement_hpp
#define GrainSVGElement_hpp

#include "Type/Object.hpp"
#include "String/String.hpp"
#include "Extern/tinyxml2.h"


namespace Grain {

    class GraphicContext;
    class SVG;
    class SVGPaintStyle;

    class SVGElement : Object {
    public:
        enum class ElementType {
            Null = 0,
            SVGRoot,
            Group,
            Defs,
            Rect,
            Circle,
            Ellipse,
            Line,
            Polyline,
            Polygon,
            Path,

            First = 0,
            Last = Path
        };

    protected:
        ElementType type_ = ElementType::Null;

        String id_;
        String class_;
        String style_;
        String language_;
        String xlink_;
        String clip_path_;
        String mask_;
        String xmlns_;
        String preserve_aspect_ratio_;

        bool valid_ = false;
        SVGElement* parent_ = nullptr;

    public:
        SVGElement(SVGElement* parent);
        ~SVGElement() {
            std::cout << "~SVGElement()\n";
        }

        friend std::ostream& operator << (std::ostream& os, const SVGElement* o) {
            o == nullptr ? os << "SVGElement nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const SVGElement& o) {
            o.log(os, 0, "SVGElement");
            return os;
        }

        virtual void log(std::ostream& os, int32_t indent, const char* label) const {}

        virtual void setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept {}


        ElementType type() const noexcept { return type_; }
        const char* typeName() const noexcept { return SVGElement::typeName(type_); }
        bool isGroup() const noexcept { return type_ == ElementType::Group; }
        bool isValid() const noexcept { return valid_; }

        const SVGElement* parent() const noexcept { return parent_; }
        SVGElement* mutableParent() const noexcept { return parent_; }


        /**
         *  @brief Check all parameters.
         */
        virtual void validate() noexcept { valid_ = false; }

        virtual bool canDraw() { return false; }

        /**
         *  @brief Draw the fill of the element to a GraphicContext.
         */
        virtual void fill(SVG* svg, GraphicContext& gc) noexcept {}

        /**
         *  @brief Draw the stroke of the element to a GraphicContext.
         */
        virtual void stroke(SVG* svg, GraphicContext& gc) noexcept {}

        virtual void parse(SVG* svg, tinyxml2::XMLElement* xml_element) {};

        virtual void draw(SVG* svg, GraphicContext& gc) noexcept {}   // Only overridden by SVGGroupElement.


        void initRootPaintStyle() noexcept;

        virtual void setPaintStyleByXMLElement(tinyxml2::XMLElement* xml_element) noexcept {}
        virtual const SVGPaintStyle* paintStyle() const noexcept { return nullptr; }
        virtual SVGPaintStyle* mutablePaintStyle() noexcept { return nullptr; }
        virtual void setCGStyle(GraphicContext& gc) const noexcept {}


        static const char* typeName(ElementType type) noexcept {
            static const char* _names[] = {
                "Null", "SVGRoot", "Group", "Defs", "Rect", "Circle", "Ellipse",
                "Line", "Polyline", "Polygon", "Path",
                "Unknown"
            };
            if (type >= ElementType::First && type <= ElementType::Last) {
                return _names[(int32_t)type];
            }
            return _names[(int32_t)ElementType::Last + 1];
        }
    };

} // End of namespace

#endif // GrainSVGElement_hpp