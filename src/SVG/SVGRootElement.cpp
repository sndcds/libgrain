//
//  SVGRootElement.cpp
//
//  Created by Roald Christesen on 03.01.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGRootElement.hpp"
#include "Core/Log.hpp"


namespace Grain {

    void SVGRootElement::log(std::ostream& os, int32_t indent, const char* label) const {
        Log l(os);
        l.header(label);
        l << "m_id: " << id_ << Log::endl;
        l << "m_class: " << class_ << Log::endl;
        l << "m_style: " << style_ << Log::endl;
        l << "m_language: " << language_ << Log::endl;
        l << "m_xlink: " << xlink_ << Log::endl;
        l << "m_clip_path: " << clip_path_ << Log::endl;
        l << "m_mask: " << mask_ << Log::endl;
        l << "m_xmlns: " << xmlns_ << Log::endl;
        l << "m_preserve_aspect_ratio: " << preserve_aspect_ratio_ << Log::endl;
        l << "m_valid: " << valid_ << Log::endl;
        l << "x_: " << x_ << ", y_: " << y_ << ", width_: " << width_ << ", height_: " << height_ << Log::endl;
        l << "viewport_x_: " << viewport_x_ << ", viewport_y_: " << viewport_y_ << ", viewport_width_: " << viewport_width_ << ", viewport_height_: " << viewport_height_ << Log::endl;
    }


    void SVGRootElement::parse(SVG* svg, tinyxml2::XMLElement* xml_element) {
        setByXMLElement(xml_element);
        initRootPaintStyle();
        setPaintStyleByXMLElement(xml_element);
        SVGGroupElement::parse(svg, xml_element);
    }


    void SVGRootElement::setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept {
        id_ = xml_element->Attribute("id");
        class_ = xml_element->Attribute("class");
        style_ = xml_element->Attribute("style");
        version_ = xml_element->Attribute("version");
        language_ = xml_element->Attribute("language");
        xlink_ = xml_element->Attribute("xlink");
        clip_path_ = xml_element->Attribute("clip_path");
        mask_ = xml_element->Attribute("mask");
        xmlns_ = xml_element->Attribute("xmlns");
        preserve_aspect_ratio_ = xml_element->Attribute("preserveAspectRatio");

        CSS::extractCSSValueFromStr(xml_element->Attribute("x"), x_, nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("y"), y_, nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("width"), width_, nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("height"), height_, nullptr);

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
                viewport_x_.setDouble(0, CSSUnit::Absolute);
                viewport_y_.setDouble(0, CSSUnit::Absolute);
                viewport_width_ = width_;
                viewport_height_ = height_;
                return;
            }

            if (value_n == 4) {
                viewport_x_.setDouble(values[0], CSSUnit::Absolute);
                viewport_y_.setDouble(values[1], CSSUnit::Absolute);
                viewport_width_.setDouble(values[2], CSSUnit::Absolute);
                viewport_height_.setDouble(values[3], CSSUnit::Absolute);
                return;
            }
        }
        catch (const Exception& e) {
            // TTodo:
        }

        std::cout << "viewport: " << viewport_x_ << ", " << viewport_y_ << ", " << viewport_width_ << ", " << viewport_height_ << std::endl;
    }
} // End of namespace

