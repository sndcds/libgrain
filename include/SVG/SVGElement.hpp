//
// SVGElement.hpp
//
// Created by Roald Christesen on 27.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
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
        ElementType m_type = ElementType::Null;

        String m_id;
        String m_class;
        String m_style;
        String m_language;
        String m_xlink;
        String m_clip_path;
        String m_mask;
        String m_xmlns;
        String m_preserve_aspect_ratio;

        bool m_valid = false;
        SVGElement* m_parent = nullptr;

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


        ElementType type() const noexcept { return m_type; }
        const char* typeName() const noexcept { return SVGElement::typeName(m_type); }
        bool isGroup() const noexcept { return m_type == ElementType::Group; }
        bool isValid() const noexcept { return m_valid; }

        const SVGElement* parent() const noexcept { return m_parent; }
        SVGElement* mutableParent() const noexcept { return m_parent; }


        /**
         *  @brief Check all parameters.
         */
        virtual void validate() noexcept { m_valid = false; }

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