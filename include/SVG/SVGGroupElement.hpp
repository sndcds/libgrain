//
//  SVGGroupElement.hpp
//
//  Created by Roald Christesen on 27.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
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
        ObjectList<SVGElement*>elements_;
        Mat3d tranformation_;

    public:
        SVGGroupElement(SVGElement* parent) : SVGPaintElement(parent) {
            type_ = ElementType::Group;
        }

        ~SVGGroupElement() {
            std::cout << "~SVGGroupElement()\n";
            elements_.clear();
        }

        void parse(SVG* svg, tinyxml2::XMLElement* xml_element) override;

        void addElement(SVGElement* element) {
            if (element != nullptr) {
                element->validate();
                elements_.push(element);
            }
        }

        void validate() noexcept override {
            valid_ = true;
        }

        void draw(SVG* svg, GraphicContext& gc) noexcept override;
    };
    
 } // End of namespace

#endif // GrainSVGGroupElement_hpp