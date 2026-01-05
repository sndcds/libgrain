//
// SVGGroupElement.cpp
//
// Created by Roald Christesen on 27.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
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
        std::cout << ".. 1\n";
        svg->incGroupIterationDepth();

        for (auto* xml_child = xml_element->FirstChildElement(); xml_child != nullptr; xml_child = xml_child->NextSiblingElement()) {
            std::cout << ".. 2\n";
            const char* tag_name = xml_child->Name();

            // std::cout << "SVGGroupElement::parse(), tag_name: " << tag_name << ", iteration depth: " << svg->groupIterationDepth() << std::endl;

            // Check for specific SVG elements

            bool can_parse = false;

            SVGElement* element = nullptr;

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

            std::cout << ".. 3\n";
            if (element) {
                addElement(element);

                std::cout << ".. 4 1\n";
                element->setByXMLElement(xml_child);
                std::cout << ".. 4 2\n";
                element->setPaintStyleByXMLElement(xml_child);

                std::cout << ".. 5\n";
                auto paint_style = element->mutablePaintStyle();
                if (paint_style != nullptr) {
                    paint_style->updateAllAttr();
                }

                std::cout << ".. 6\n";
                if (can_parse == true) {
                    element->parse(svg, xml_child);
                }
            }
        }

        std::cout << ".. 7\n";
        svg->decGroupIterationDepth();
        std::cout << ".. 8\n";
    }


    // TODO: Transformation ... Mat3d m_tranformation;
    void SVGGroupElement::draw(SVG* svg, GraphicContext& gc) noexcept {
        for (auto element : m_elements) {
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