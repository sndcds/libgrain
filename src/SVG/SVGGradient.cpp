//
// SVGGradient.cpp
//
// Created by Roald Christesen on 11.01.2025
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGGradient.hpp"
#include "Core/Log.hpp"


namespace Grain {

    void SVGGradientColorStop::setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept {
        CSS::extractCSSValueFromStr(xml_element->Attribute("offset"), m_offset, nullptr);

        auto err = m_color.setByCSS(xml_element->Attribute("stop-color"));

        std::cout << "> > > > stop-color\n";
        if (xml_element->Attribute("stop-color") != nullptr) {
            std::cout << "    " << xml_element->Attribute("stop-color") << std::endl;
            std::cout << "    " << m_color << std::endl;
        }

        // TODO: Handle error!
    }


    /**
     *  @brief Constructor to initialize the gradient with a specific type.
     *
     *  @param type The type of gradient (Linear or Radial).
     */
    SVGGradient::SVGGradient(SVGGradientType type, int32_t capacity) noexcept {
        m_server_type = PaintServerType::Gradient;
        m_color_stops.reserve(std::max(capacity, 4));
    }


    SVGGradient::~SVGGradient() noexcept {
    }


    void SVGGradient::log(std::ostream& os, int32_t indent, const char* label) const {
        Log l(os);

        l.header(label);

        l << "m_id: " << m_id << Log::endl;
        l << "m_class: " << m_class << Log::endl;
        l << "m_style: " << m_style << Log::endl;
        l << "m_href " << m_href << Log::endl;

        l << "gradientTypeName() " << SVG::gradientTypeName(m_gradient_type) << Log::endl;
        l << "gradientInterpolationModeName() " << SVG::gradientInterpolationModeName(m_color_interpolation_mode) << Log::endl;
        l << "gradientUnitsName() " << SVG::gradientUnitsName(m_units) << Log::endl;
        l << "m_href " << m_href << Log::endl;

        if (m_gradient_type == SVGGradientType::Linear) {
            l << "x1: " << m_values[kValueX1];
            l << ", y1: " << m_values[kValueY1];
            l << ", x2: " << m_values[kValueX2];
            l << ", y2: " << m_values[kValueY2] << Log::endl;
        }
        else if (m_gradient_type == SVGGradientType::Radial) {
            l << "x1: " << m_values[kValueCX];
            l << ", cy: " << m_values[kValueCY];
            l << ", r: " << m_values[kValueR];
            l << ", fx: " << m_values[kValueFX];
            l << ", fy: " << m_values[kValueFY] << Log::endl;
        }

        l << "m_color_stops.size(): " << m_color_stops.size() << Log::endl;

        l++;
        int32_t index = 0;
        for (auto& stop : m_color_stops) {
            l << index << ": offset: " << stop->m_offset;
            l << index << ": color: " << stop->m_color << Log::endl;
            index++;
        }
    }


    /**
     *  @brief Sets the color interpolation mode for the gradient.
     *
     *  @param mode The interpolation mode.
     */
    void SVGGradient::setColorInterpolation(SVGGradientInterpolationMode mode) noexcept {

        m_color_interpolation_mode = mode;
    }


    /**
     * @brief Sets the transformation for the gradient.
     * @param transform The transformation matrix as a string.
     */
    void SVGGradient::setTransform(const char* transform) noexcept {

    }


    /**
     *  @brief Adds a color stop to the gradient.
     *
     *  @param offset The position of the stop (0.0 to 1.0).
     *  @param color The color at the stop.
     */
    void SVGGradient::addColorStop(float offset, const RGBA& color) noexcept {

    }


    void SVGGradient::addColorStop(SVG* svg, tinyxml2::XMLElement* xml_element) {

        svg->incGroupIterationDepth();

        auto color_stop = new(std::nothrow) SVGGradientColorStop();
        if (color_stop) {
            m_color_stops.push(color_stop);
            color_stop->setByXMLElement(xml_element);
        }

        svg->decGroupIterationDepth();
    }


    // TODO: try/catch/err
    void SVGGradient::parse(SVG* svg, tinyxml2::XMLElement* xml_element) {
        svg->incGroupIterationDepth();

        setByXMLElement(xml_element);

        for (auto* xml_child = xml_element->FirstChildElement(); xml_child != nullptr; xml_child = xml_child->NextSiblingElement()) {
            const char* tag_name = xml_child->Name();

            if (SVG::isTag(tag_name, "stop")) {
                addColorStop(svg, xml_child);
            }

            // Check for specific SVG elements.
        }


        for (auto& stop : m_color_stops) {
           m_grain_gradient.addStop(stop->m_offset.valueAsDoubleConsiderPercentage(), stop->m_color);
        }

        switch (m_color_interpolation_mode) {
            case SVGGradientInterpolationMode::LinearRGB:
                m_grain_gradient.setColorSpace(Gradient::ColorSpace::LinearRGB);
                break;
            case SVGGradientInterpolationMode::sRGB:
            default:
                m_grain_gradient.setColorSpace(Gradient::ColorSpace::sRGB);
                break;
        }

        log(std::cout, 0, "SVGGradient");

        svg->decGroupIterationDepth();
    }


    void SVGGradient::setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept {

        m_id = xml_element->Attribute("id");
        m_class = xml_element->Attribute("class");
        m_style = xml_element->Attribute("style");
        m_href = xml_element->Attribute("href");

        {
            auto attr_str = xml_element->Attribute("color-interpolation");
            if (attr_str != nullptr) {
                if (strcasecmp(attr_str, "sRGB") == 0) {
                    m_color_interpolation_mode = SVGGradientInterpolationMode::sRGB;
                }
                else if (strcasecmp(attr_str, "linearRGB") == 0) {
                    m_color_interpolation_mode = SVGGradientInterpolationMode::LinearRGB;
                }
                else {
                    // TODO: Warning message!
                }
            }
        }


        // xml_element->Attribute("color-interpolation");


        // TODO: Optional Attributes
        //   href (or xlink:href in older SVGs): References another <linearGradient> to inherit properties.

        CSS::extractCSSValueFromStr(xml_element->Attribute("x1"), m_values[kValueX1], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("y1"), m_values[kValueY1], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("x2"), m_values[kValueX2], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("y2"), m_values[kValueY2], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("cx"), m_values[kValueCX], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("cy"), m_values[kValueCY], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("r"), m_values[kValueR], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("fy"), m_values[kValueFX], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("fy"), m_values[kValueFY], nullptr);
    }


    /**
     * @brief Generates the SVG representation of the gradient.
     * @return A string containing the SVG markup for the gradient.
     */
    ErrorCode SVGGradient::toSVG(String& out_svg) const noexcept {

        // TODO: Implement!
        return ErrorCode::None;
    }

} // End of namespace