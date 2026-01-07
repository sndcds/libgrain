//
//  SVGGroupElement.cpp
//
//  Created by Roald Christesen on 27.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGGroupElement.hpp"
#include "SVG/SVGDefsElement.hpp"
#include "SVG/SVGRectElement.hpp"
#include "SVG/SVGCircleElement.hpp"
#include "SVG/SVGEllipseElement.hpp"
#include "SVG/SVGLineElement.hpp"
#include "SVG/SVGPolygonElement.hpp"
#include "SVG/SVGPathElement.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    // TODO: try/catch/err
    void SVGGroupElement::parse(SVG* svg, tinyxml2::XMLElement* xml_element) {
        svg->incGroupIterationDepth();

        for (auto* xml_child = xml_element->FirstChildElement(); xml_child != nullptr; xml_child = xml_child->NextSiblingElement()) {
            const char* tag_name = xml_child->Name();

            std::cout << "SVGGroupElement::parse(), tag_name: " << tag_name << ", iteration depth: " << svg->groupIterationDepth() << std::endl;
            // Check for specific SVG elements

            SVGElement* element = nullptr;
            bool can_parse = false;

            if (SVG::isTag(tag_name, "g")) {
                auto group_element = new(std::nothrow) SVGGroupElement(this);
                if (!group_element) { Exception::throwStandard(ErrorCode::MemCantAllocate); }
                element = group_element;
                can_parse = true;
            }
            else if (SVG::isTag(tag_name, "defs")) {
                auto defs_element = new(std::nothrow) SVGDefsElement(this);
                if (!defs_element) { Exception::throwStandard(ErrorCode::MemCantAllocate); }
                element = defs_element;
                can_parse = true;
            }
            else if (SVG::isTag(tag_name, "rect")) {
                element = new(std::nothrow) SVGRectElement(this);
                if (!element) { Exception::throwStandard(ErrorCode::MemCantAllocate); }
            }
            else if (SVG::isTag(tag_name, "circle")) {
                element = new(std::nothrow) SVGCircleElement(this);
                if (!element) { Exception::throwStandard(ErrorCode::MemCantAllocate); }
            }
            else if (SVG::isTag(tag_name, "ellipse")) {
                element = new(std::nothrow) SVGEllipseElement(this);
                if (!element) { Exception::throwStandard(ErrorCode::MemCantAllocate); }
            }
            else if (SVG::isTag(tag_name, "line")) {
                element = new(std::nothrow) SVGLineElement(this);
                if (!element) { Exception::throwStandard(ErrorCode::MemCantAllocate); }
            }
            else if (SVG::isTag(tag_name, "polyline")) {
                auto polyline_element = new(std::nothrow) SVGPolygonElement(this);
                if (!polyline_element) {
                    Exception::throwStandard(ErrorCode::MemCantAllocate);
                }
                else {
                    const char* data = xml_child->Attribute("points");
                    polyline_element->parseData(svg, data);
                    element = polyline_element;
                }
            }
            else if (SVG::isTag(tag_name, "polygon")) {
                auto polygon_element = new(std::nothrow) SVGPolygonElement(this);
                if (!polygon_element) {
                    Exception::throwStandard(ErrorCode::MemCantAllocate);
                }
                else {
                    const char* data = xml_child->Attribute("points");
                    polygon_element->parseData(svg, data);
                    polygon_element->close();
                    element = polygon_element;
                }
            }
            else if (SVG::isTag(tag_name, "path")) {
                auto path_element = new(std::nothrow) SVGPathElement(this);
                if (!path_element) { Exception::throwStandard(ErrorCode::MemCantAllocate); }
                const char* data = xml_child->Attribute("d");
                path_element->parsePathData(svg, data);
                element = path_element;
            }
            else {
                std::cout << "SVGGroupElement::parse() unknown tag: " << tag_name << std::endl;
                // TODO: What should happen with unknown child types?
            }

            if (element) {
                addElement(element);

                element->setByXMLElement(xml_child);
                element->setPaintStyleByXMLElement(xml_child);

                auto paint_style = element->mutablePaintStyle();
                if (paint_style) {
                    paint_style->updateAllAttr();
                }

                if (can_parse) {
                    element->parse(svg, xml_child);
                }
            }
        }

        svg->decGroupIterationDepth();
    }


    // TODO: Transformation ... Mat3d m_tranformation;
    void SVGGroupElement::draw(SVG* svg, GraphicContext& gc) noexcept {
        for (auto element : elements_) {
            if (canDraw()) {
                auto paint_element = (SVGPaintElement*)element;
                gc.save();

                paint_element->setCGStyle(gc);

                if (paint_element->isGroup()) {
                    paint_element->draw(svg, gc);
                }
                else {
                    if (paint_element->doesFill()) {
                        paint_element->fill(svg, gc);
                    }

                    if (paint_element->doesStroke()) {
                        paint_element->stroke(svg, gc);
                    }
                }

                gc.restore();
            }
        }
    }

} // End of namespace