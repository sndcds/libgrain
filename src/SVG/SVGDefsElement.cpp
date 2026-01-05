//
// SVGDefsElement.cpp
//
// Created by Roald Christesen on 03.01.2025
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGDefsElement.hpp"
#include "SVG/SVG.hpp"
#include "SVG/SVGGradient.hpp"


namespace Grain {

    // Todo: try/catch/err
    void SVGDefsElement::parse(SVG* svg, tinyxml2::XMLElement* xml_element) {
        svg->incGroupIterationDepth();

        for (auto* xml_child = xml_element->FirstChildElement();
            xml_child != nullptr;
            xml_child = xml_child->NextSiblingElement())
        {
            const char* tag_name = xml_child->Name();

            // std::cout << "SVGGroupElement::parse(), tag_name: " << tag_name << ", iteration depth: " << svg->groupIterationDepth() << std::endl;
            // Todo: Check for specific SVG elements.

            if (SVG::isTag(tag_name, "linearGradient")) {
                auto gradient = svg->addGradient(SVGGradientType::Linear);
                if (!gradient) {
                    Exception::throwSpecific(SVG::kErrAddGradientFailed);
                }
                else {
                    gradient->parse(svg, xml_child);
                }
            }
            else if (SVG::isTag(tag_name, "radialGradient")) {
                auto gradient = svg->addGradient(SVGGradientType::Radial);
                if (!gradient) {
                    Exception::throwSpecific(SVG::kErrAddGradientFailed);
                }
            }
            else if (SVG::isTag(tag_name, "pattern")) {
            }
            else if (SVG::isTag(tag_name, "filter")) {
            }
            else if (SVG::isTag(tag_name, "clipPath")) {
            }
            else if (SVG::isTag(tag_name, "mask")) {
            }
            else if (SVG::isTag(tag_name, "marker")) {
            }
            else if (SVG::isTag(tag_name, "symbol")) {
            }
            else if (SVG::isTag(tag_name, "use")) {
            }
            else if (SVG::isTag(tag_name, "marker")) {
            }

            // TODO: <rect>, <circle>, <ellipse>, <line>, <polygon>, <polyline>, <path>, <text>, <textPath>, <font>, <glyph>, <animate>, <animateTransform>, <set>, <style>, <script>, <metadata>
        }

        svg->decGroupIterationDepth();
    }


} // End of namespace