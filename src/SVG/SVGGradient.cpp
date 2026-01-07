//
//  SVGGradient.cpp
//
//  Created by Roald Christesen on 11.01.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGGradient.hpp"
#include "Core/Log.hpp"


namespace Grain {

    void SVGGradientColorStop::setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept {
        CSS::extractCSSValueFromStr(xml_element->Attribute("offset"), offset_, nullptr);

        auto err = color_.setByCSS(xml_element->Attribute("stop-color"));

        std::cout << "> > > > stop-color\n";
        if (xml_element->Attribute("stop-color") != nullptr) {
            std::cout << "    " << xml_element->Attribute("stop-color") << std::endl;
            std::cout << "    " << color_ << std::endl;
        }

        // TODO: Handle error!
    }


    /**
     *  @brief Constructor to initialize the gradient with a specific type.
     *
     *  @param type The type of gradient (Linear or Radial).
     */
    SVGGradient::SVGGradient(SVGGradientType type, int32_t capacity) noexcept {
        server_type_ = PaintServerType::Gradient;
        color_stops_.reserve(std::max(capacity, 4));
    }


    SVGGradient::~SVGGradient() noexcept {
    }


    void SVGGradient::log(std::ostream& os, int32_t indent, const char* label) const {
        Log l(os);

        l.header(label);

        l << "id_: " << id_ << Log::endl;
        l << "class_: " << class_ << Log::endl;
        l << "style_: " << style_ << Log::endl;
        l << "href_ " << href_ << Log::endl;

        l << "gradientTypeName() " << SVG::gradientTypeName(gradient_type_) << Log::endl;
        l << "gradientInterpolationModeName() " << SVG::gradientInterpolationModeName(color_interpolation_mode_) << Log::endl;
        l << "gradientUnitsName() " << SVG::gradientUnitsName(units_) << Log::endl;
        l << "href_ " << href_ << Log::endl;

        if (gradient_type_ == SVGGradientType::Linear) {
            l << "x1: " << values_[kValueX1];
            l << ", y1: " << values_[kValueY1];
            l << ", x2: " << values_[kValueX2];
            l << ", y2: " << values_[kValueY2] << Log::endl;
        }
        else if (gradient_type_ == SVGGradientType::Radial) {
            l << "x1: " << values_[kValueCX];
            l << ", cy: " << values_[kValueCY];
            l << ", r: " << values_[kValueR];
            l << ", fx: " << values_[kValueFX];
            l << ", fy: " << values_[kValueFY] << Log::endl;
        }

        l << "color_stops_.size(): " << color_stops_.size() << Log::endl;

        l++;
        int32_t index = 0;
        for (auto& stop : color_stops_) {
            l << index << ": offset: " << stop->offset_;
            l << index << ": color: " << stop->color_ << Log::endl;
            index++;
        }
    }


    /**
     *  @brief Sets the color interpolation mode for the gradient.
     *
     *  @param mode The interpolation mode.
     */
    void SVGGradient::setColorInterpolation(SVGGradientInterpolationMode mode) noexcept {
        color_interpolation_mode_ = mode;
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
            color_stops_.push(color_stop);
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

        for (auto& stop : color_stops_) {
           grain_gradient_.addStop(stop->offset_.valueAsDoubleConsiderPercentage(), stop->color_);
        }

        switch (color_interpolation_mode_) {
            case SVGGradientInterpolationMode::LinearRGB:
                grain_gradient_.setColorSpace(Gradient::ColorSpace::LinearRGB);
                break;
            case SVGGradientInterpolationMode::sRGB:
            default:
                grain_gradient_.setColorSpace(Gradient::ColorSpace::sRGB);
                break;
        }

        log(std::cout, 0, "SVGGradient");

        svg->decGroupIterationDepth();
    }


    void SVGGradient::setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept {
        id_ = xml_element->Attribute("id");
        class_ = xml_element->Attribute("class");
        style_ = xml_element->Attribute("style");
        href_ = xml_element->Attribute("href");

        {
            auto attr_str = xml_element->Attribute("color-interpolation");
            if (attr_str != nullptr) {
                if (strcasecmp(attr_str, "sRGB") == 0) {
                    color_interpolation_mode_ = SVGGradientInterpolationMode::sRGB;
                }
                else if (strcasecmp(attr_str, "linearRGB") == 0) {
                    color_interpolation_mode_ = SVGGradientInterpolationMode::LinearRGB;
                }
                else {
                    // TODO: Warning message!
                }
            }
        }


        // xml_element->Attribute("color-interpolation");


        // TODO: Optional Attributes
        //   href (or xlink:href in older SVGs): References another <linearGradient> to inherit properties.

        CSS::extractCSSValueFromStr(xml_element->Attribute("x1"), values_[kValueX1], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("y1"), values_[kValueY1], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("x2"), values_[kValueX2], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("y2"), values_[kValueY2], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("cx"), values_[kValueCX], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("cy"), values_[kValueCY], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("r"), values_[kValueR], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("fy"), values_[kValueFX], nullptr);
        CSS::extractCSSValueFromStr(xml_element->Attribute("fy"), values_[kValueFY], nullptr);
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