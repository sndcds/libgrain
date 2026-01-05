//
// SVGRootElement.cpp
//
// Created by Roald Christesen on 03.01.2025
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGRootElement.hpp"
#include "SVG/SVG.hpp"
#include "Core/Log.hpp"


namespace Grain {

    void SVGRootElement::log(std::ostream& os, int32_t indent, const char* label) const {
        Log l(os);
        l.header(label);
        l << "m_id: " << m_id << Log::endl;
        l << "m_class: " << m_class << Log::endl;
        l << "m_style: " << m_style << Log::endl;
        l << "m_language: " << m_language << Log::endl;
        l << "m_xlink: " << m_xlink << Log::endl;
        l << "m_clip_path: " << m_clip_path << Log::endl;
        l << "m_mask: " << m_mask << Log::endl;
        l << "m_xmlns: " << m_xmlns << Log::endl;
        l << "m_preserve_aspect_ratio: " << m_preserve_aspect_ratio << Log::endl;
        l << "m_valid: " << m_valid << Log::endl;
        l << "m_x: " << m_x << ", m_y: " << m_y << ", m_width: " << m_width << ", m_height: " << m_height << Log::endl;
        l << "m_viewport_x: " << m_viewport_x << ", m_viewport_y: " << m_viewport_y << ", m_viewport_width: " << m_viewport_width << ", m_viewport_height: " << m_viewport_height << Log::endl;
    }


    void SVGRootElement::parse(SVG* svg, tinyxml2::XMLElement* xml_element) {
        setByXMLElement(xml_element);
        initRootPaintStyle();
        setPaintStyleByXMLElement(xml_element);
        SVGGroupElement::parse(svg, xml_element);
    }


    void SVGRootElement::setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept {
        m_id = xml_element->Attribute("id");
        m_class = xml_element->Attribute("class");
        m_style = xml_element->Attribute("style");
        m_version = xml_element->Attribute("version");
        m_language = xml_element->Attribute("language");
        m_xlink = xml_element->Attribute("xlink");
        m_clip_path = xml_element->Attribute("clip_path");
        m_mask = xml_element->Attribute("mask");
        m_xmlns = xml_element->Attribute("xmlns");
        m_preserve_aspect_ratio = xml_element->Attribute("preserveAspectRatio");

        CSS::extractCSSValueFromStr(xml_element->Attribute("x"), m_x, nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("y"), m_y, nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("width"), m_width, nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("height"), m_height, nullptr);

        setViewBox(xml_element->Attribute("viewBox"));
    }


    void SVGRootElement::setViewBox(const char* str) noexcept {
        try {
            if (!str) {
                Exception::throwStandard(ErrorCode::NullData);
            }

            char buffer[256];
            float values[4];

            int32_t value_n = CSS::extractValuesFromStr(str, buffer, 256, '\0', 4, values);
            if (value_n == 0) {
                m_viewport_x.setDouble(0, CSSUnit::Absolute);
                m_viewport_y.setDouble(0, CSSUnit::Absolute);
                m_viewport_width = m_width;
                m_viewport_height = m_height;
                return;
            }

            if (value_n == 4) {
                m_viewport_x.setDouble(values[0], CSSUnit::Absolute);
                m_viewport_y.setDouble(values[1], CSSUnit::Absolute);
                m_viewport_width.setDouble(values[2], CSSUnit::Absolute);
                m_viewport_height.setDouble(values[3], CSSUnit::Absolute);
                return;
            }
        }
        catch (const Exception& e) {
            // TODO:
        }

    }
} // End of namespace

