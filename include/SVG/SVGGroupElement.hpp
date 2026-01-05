//
// SVGGroupElement.hpp
//
// Created by Roald Christesen on 27.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGGroupElement_hpp
#define GrainSVGGroupElement_hpp

#include "SVG/SVGPaintElement.hpp"
#include "Type/List.hpp"
#include "Math/Mat3.hpp"


namespace Grain {

    class SVG;
    class GraphicContext;

    class SVGGroupElement : public SVGPaintElement {
    protected:
        ObjectList<SVGElement*>m_elements;
        Mat3d m_tranformation;

    public:
        SVGGroupElement(SVGElement* parent) : SVGPaintElement(parent) {
            m_type = ElementType::Group;
        }

        ~SVGGroupElement() {
            std::cout << "~SVGGroupElement()\n";
            m_elements.clear();
        }

        void parse(SVG* svg, tinyxml2::XMLElement* xml_element) override;

        void addElement(SVGElement* element) {
            if (element != nullptr) {
                element->validate();
                m_elements.push(element);
            }
        }

        void validate() noexcept override {
            m_valid = true;
        }

        void draw(SVG* svg, GraphicContext& gc) noexcept override;
    };
    
 } // End of namespace

#endif // GrainSVGGroupElement_hpp