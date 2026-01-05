//
// SVGPaintStyle.cpp
//
// Created by Roald Christesen on 03.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGPaintStyle.hpp"
#include "SVG/SVG.hpp"
#include "SVG/SVGElement.hpp"
#include "CSS/CSS.hpp"
#include "CSS/CSSColor.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Core/Log.hpp"


namespace Grain {

    const SVGNumericAttrKeyValue SVGPaintStyle::_g_fill_rule_table[] = {
        { "nonzero", (int64_t)FillWindingRule::NoneZero },
        { "evenodd", (int64_t)FillWindingRule::EvenOdd },
        { "", -1 }
    };

    const SVGNumericAttrKeyValue SVGPaintStyle::_g_stroke_linecap_table[] = {
        { "butt", (int64_t)StrokeCapStyle::Butt },
        { "round", (int64_t)StrokeCapStyle::Round },
        { "square", (int64_t)StrokeCapStyle::Square },
        { "", -1 }
    };

    const SVGNumericAttrKeyValue SVGPaintStyle::_g_stroke_linejoin_table[] = {
        { "miter", (int64_t)StrokeJoinStyle::Miter },
        { "round", (int64_t)StrokeJoinStyle::Round },
        { "bevel", (int64_t)StrokeJoinStyle::Bevel },
        { "", -1 }
    };


    void SVGAttr::initWithFlags(uint32_t attr_flags) noexcept {
        m_can_initial = (attr_flags & kFlag_CanInitial) != 0;
        m_can_unset = (attr_flags & kFlag_CanUnset) != 0;
        m_can_inherit = (attr_flags & kFlag_CanInherit) != 0;
        m_can_be_none = (attr_flags & kFlag_CanBeNone) != 0;
    };


    void SVGAttr::set(const char* str) noexcept {
        m_has_value = false;
        m_must_update = true;
        m_is_none = false;
        m_set_command = SetCommand::DoNothing;

        if (str == nullptr) {
            return;
        }

        if (canBeNone() && (strcasecmp(str, "none") == 0 || strcasecmp(str, "transparent") == 0)) {
            if (canInherit()) {
                m_set_command = SetCommand::SetByInheritance;
            }
            m_has_value = true;
            m_is_none = true;
        }

        if (canInherit() && strcasecmp(str, "inherit") == 0) {
            m_set_command = SetCommand::SetByInheritance;
        }

        if (canInitial() && strcasecmp(str, "initial") == 0) {
            m_set_command = SetCommand::SetByInitialValue;
        }

        if (canUnset() && strcasecmp(str, "unset") == 0) {
            if (canInherit()) {
                // Equivalent to inherit.
                m_set_command = SetCommand::SetByInheritance;
            }
            else {
                // Equivalent to initial.
                m_set_command = SetCommand::SetByInitialValue;
            }
        }
    }


    /**
     *  @brief Sets the color value for an SVG attribute based on a string input.
     *
     *  This method processes a string to set the color of an SVG attribute, supporting
     *  CSS color values, special keywords (`inherit`, `initial`, `unset`), and the
     *  attribute's inheritance rules.
     *
     *  @param str The input string represattr_flagsenting the color value. This can be a valid CSS
     *             color string, or the keywords "inherit", "initial", or "unset".
     *  @param initial_color The color value to use when the `initial` or `unset` keyword resolves
     *                       to the attribute's initial value.
     *
     *  @return `true` if the color was successfully set, `false` if the input string was invalid.
     */
    bool SVGColorAttr::setColor(const char* str, const RGBA& initial_color) noexcept {
        m_use_current_color = false;

        str = String::firstNonWhiteSpaceCharPtr(str);

        SVGAttr::set(str);

        if (m_set_command == SetCommand::SetByInitialValue) {
            m_color = initial_color;
            m_has_value = true;
            return true;
        }

        if (m_set_command == SetCommand::SetByInheritance) {
            // Will be handled later.
            return true;
        }

        if (strcasecmp(str, "currentColor") == 0) {
            m_set_command = SetCommand::SetToCurrentColor;
            m_has_value = true;
            return true;
        }

        if (strncasecmp(str, "url(", 4) == 0) {
            int64_t n = m_raw.setByFramedContent(str, '(', ')');
            if (n >= 0) {
                m_draw_mode = DrawMode::Gradient;
                m_set_command = SetCommand::SetToURL;
                m_has_value = true;
                return true;
            }
        }

        auto err = CSSColor::parseColorToRGBA(str, m_color);
        if (Error::isError(err)) {
            return false;
        }
        else {
            m_has_value = true;
            return true;
        }
    }


    bool SVGNumericAttr::setValue(const char* str, const CSSValue& initial_value) noexcept {
        str = String::firstNonWhiteSpaceCharPtr(str);

        SVGAttr::set(str);

        if (m_set_command == SetCommand::SetByInitialValue) {
            m_css_value = initial_value;
            m_has_value = true;
            return true;
        }

        if (m_set_command == SetCommand::SetByInheritance) {
            // Will be handled later.
            return true;
        }

        // Key/Value table.

        if (m_key_value_table != nullptr) {
            int32_t index = 0;
            while (m_key_value_table[index].m_key[0] != '\0') {
                if (strcasecmp(str, m_key_value_table[index].m_key) == 0) {
                    m_has_value = true;
                    m_css_value.setInt32Absolute((int32_t)m_key_value_table[index].m_value);
                    return true;
                }
                index++;
            }
        }

        auto err = CSS::extractCSSValueFromStr(str, m_css_value, nullptr);
        if (Error::isError(err)) {
            return false;
        }
        else {
            m_has_value = true;
            return true;
        }
    }


    /**
     *  @brief:
     */
    void SVGPaintStyle::updateNumericAttr(AttrID attr_id, SVGNumericAttr* out_attr) noexcept {
        auto attr = numericAttrByID(attr_id);
        if (attr == nullptr) {
            _m_fatal_error_count++;
            return;
        }

        if (attr->mustUpdate()) {
            if (attr->hasValue() == false && attr->canInherit() == true) {
                auto parent = m_svg_element_ptr->parent();
                if (parent != nullptr) {
                    auto parent_paint_style = (SVGPaintStyle*)parent->paintStyle();
                    if (parent_paint_style != nullptr) {
                        parent_paint_style->updateNumericAttr(attr_id, attr);
                    }
                    else {
                        _m_fatal_error_count++;
                    }
                }
            }

            attr->m_must_update = false;
        }

        if (out_attr != nullptr) {
            out_attr->m_css_value = attr->m_css_value;
            out_attr->m_has_value = attr->m_has_value;
            out_attr->m_is_none = attr->m_is_none;
        }
    }


    /**
     *  @brief:
     */
    void SVGPaintStyle::updateColorAttr(AttrID attr_id, SVGColorAttr* out_attr) noexcept {
        auto attr = colorAttrByID(attr_id);
        if (attr == nullptr) {
            _m_fatal_error_count++;
            return;
        }

        if (attr->mustUpdate() == true) {

            if (attr->hasValue() == false && attr->canInherit() == true) {
                auto parent = m_svg_element_ptr->parent();
                if (parent != nullptr) {
                    auto parent_paint_style = (SVGPaintStyle*)parent->paintStyle();
                    if (parent_paint_style != nullptr) {
                        parent_paint_style->updateColorAttr(attr_id, attr);
                    }
                    else {
                        _m_fatal_error_count++;
                    }
                }
            }


            attr->m_must_update = false;
        }

        if (out_attr != nullptr) {
            out_attr->m_color = attr->m_color;
            out_attr->m_use_current_color = attr->m_use_current_color;
            out_attr->m_has_value = attr->m_has_value;
            out_attr->m_is_none = attr->m_is_none;
        }
    }


    void SVGPaintStyle::setGCSettings(GraphicContext& gc) const noexcept {
        if (m_transform_count > 0) {
            for (int32_t i = 0; i < m_transform_count; i++) {
                m_transform_stack[i].transformGC(gc);
            }
        }

        if (m_attr_opacity.hasValue()) {
            gc.setAlpha(m_attr_opacity.valueAsDouble());
        }

        if (m_attr_fill.hasValue()) {

            if (m_attr_fill.m_use_current_color) {
                gc.setFillRGB(m_attr_color.m_color);
            }
            else {
                gc.setFillRGB(m_attr_fill.m_color);
            }
        }

        if (m_attr_stroke.hasValue()) {
            if (m_attr_stroke.m_use_current_color) {
                gc.setStrokeRGB(m_attr_color.m_color);
            }
            else {
                gc.setStrokeRGB(m_attr_stroke.m_color);
            }
        }

        if (m_attr_stroke_width.hasValue()) {
            gc.setStrokeWidth(m_attr_stroke_width.m_css_value.valueAsDouble());  // TODO: Handle unit.
        }

        if (m_attr_stroke_linecap.hasValue()) {
            gc.setStrokeCapStyle((StrokeCapStyle)m_attr_stroke_linecap.m_css_value.valueAsInt32());
        }

        if (m_attr_stroke_linejoin.hasValue()) {
            gc.setStrokeJoinStyle((StrokeJoinStyle)m_attr_stroke_linejoin.m_css_value.valueAsInt32());
        }

        if (m_attr_stroke_miterlimit.hasValue()) {
            gc.setStrokeMiterLimit(m_attr_stroke_miterlimit.m_css_value.valueAsDouble());
        }
    }


    /**
     *  @brief Set all paint style properties with values from `xml_element`.
     */
    void SVGPaintStyle::setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept {
        std::cout << ".. a\n";
        if (xml_element != nullptr) {
            CSSValue default_css_value; // Is undefined by default.
            RGBA default_color = { 0.0f, 0.0f, 0.0f, 1.0f };

        std::cout << ".. b\n";
            // Iterate over all attributes of the element.

            const tinyxml2::XMLAttribute* attribute = xml_element->FirstAttribute();
        std::cout << ".. c\n";
            while (attribute != nullptr) {
                auto name = attribute->Name();
                auto value = attribute->Value();
                std::cout << ".. d\n";
                std::cout << "name: " << name << ", value: " << value << std::endl;

                bool set_result = false;

                if (strcasecmp(name, "opacity") == 0) {
                    set_result = m_attr_opacity.setValue(value, default_css_value);
                }
                else if (strcasecmp(name, "color") == 0) {
                    set_result = m_attr_color.setColor(value, default_color);
                }
                else if (strcasecmp(name, "fill") == 0) {
                    set_result = m_attr_fill.setColor(value, default_color);
                }
                else if (strcasecmp(name, "fill-rule") == 0) {
                    set_result = m_attr_fill_rule.setValue(xml_element->Attribute("fill-rule"), default_css_value);
                }
                else if (strcasecmp(name, "fill-opacity") == 0) {
                    set_result = m_attr_fill_opacity.setValue(xml_element->Attribute("fill-rule"), default_css_value);
                }
                else if (strcasecmp(name, "stroke") == 0) {
                    set_result = m_attr_stroke.setColor(value, default_color);
                }
                else if (strcasecmp(name, "stroke-width") == 0) {
                    set_result = m_attr_stroke_width.setValue(value, default_css_value);
                }
                else if (strcasecmp(name, "stroke-linecap") == 0) {
                    set_result = m_attr_stroke_linecap.setValue(value, default_css_value);
                }
                else if (strcasecmp(name, "stroke-linejoin") == 0) {
                    set_result = m_attr_stroke_linecap.setValue(value, default_css_value);
                }
                else if (strcasecmp(name, "stroke-miterlimit") == 0) {
                    set_result = m_attr_stroke_miterlimit.setValue(value, default_css_value);
                }
                else if (strcasecmp(name, "stroke-opacity") == 0) {
                    set_result = m_attr_stroke_opacity.setValue(value, default_css_value);
                }
                else if (strcasecmp(name, "transform") == 0) {
                    parseTransform(value);
                }
                std::cout << ".. e\n";

                if (set_result != true) {
                    // TODO: Warning or Error Message!!!!!
                }

                attribute = attribute->Next();
            }

            /* TODO: !!!!
            m_stroke_dasharray = SVG::_validStr(e->Attribute("stroke-dasharray"));
            m_stroke_dashoffset = SVG::_validStr(e->Attribute("stroke-dashoffset"));
             */
        std::cout << ".. f\n";
        }
        std::cout << ".. g\n";

    }


    void SVGPaintStyle::setDefault() noexcept {
        m_attr_opacity.initWithFlags(SVGAttr::kFlags_Default);
        m_attr_opacity.setDoubleAbsolute(1.0);
        m_attr_opacity.setMinMax(0, 1);

        // Color
        m_attr_color.initWithFlags(SVGAttr::kFlags_Default);
        m_attr_color.setColor(RGBA(0.0f, 0.0f, 0.0f, 1.0f));

        // Fill
        m_attr_fill.initWithFlags(SVGAttr::kFlags_Default);
        m_attr_fill.setColor(RGBA(0.0f, 0.0f, 0.0f, 1.0f));

        m_attr_fill_rule.setKeyValueTable(_g_fill_rule_table);
        m_attr_fill_rule.initWithFlags(SVGAttr::kFlags_Default);
        m_attr_fill_rule.setInt32Absolute((int32_t)FillWindingRule::NoneZero);

        m_attr_fill_opacity.initWithFlags(SVGAttr::kFlags_Default);
        m_attr_fill_opacity.setDoubleAbsolute(1.0);
        m_attr_fill_opacity.setMinMax(0, 1);

        // Stroke
        m_attr_stroke.initWithFlags(SVGAttr::kFlags_Default);

        m_attr_stroke_width.initWithFlags(SVGAttr::kFlags_Default);

        m_attr_stroke_linecap.setKeyValueTable(_g_stroke_linecap_table);
        m_attr_stroke_linecap.initWithFlags(SVGAttr::kFlags_Default);
        m_attr_stroke_linecap.setInt32Absolute((int32_t)StrokeCapStyle::Butt);

        m_attr_stroke_linejoin.setKeyValueTable(_g_stroke_linejoin_table);
        m_attr_stroke_linejoin.initWithFlags(SVGAttr::kFlags_Default);
        m_attr_stroke_linejoin.setInt32Absolute((int32_t)StrokeJoinStyle::Miter);

        m_attr_stroke_miterlimit.initWithFlags(SVGAttr::kFlags_Default);
        m_attr_stroke_miterlimit.setMinMax(1, 10);

        m_attr_stroke_opacity.initWithFlags(SVGAttr::kFlags_Default);
        m_attr_stroke_opacity.setDoubleAbsolute(1.0);
        m_attr_stroke_opacity.setMinMax(0, 1);

        m_has_fill_opacity = false;
        m_has_stroke_linecap = false;
        m_has_stroke_linejoin = false;
        m_has_stroke_miterlimit = false;
        m_has_stroke_opacity = false;

        m_does_fill = false;
        m_does_stroke = false;

        // TODO: stroke-dasharray none
        // TODO: stroke-dashoffset 0
    }


    ErrorCode SVGPaintStyle::parseTransform(const char* str) noexcept {

std::cout << "..... 1\n";
        auto result = ErrorCode::None;

        try {
std::cout << "..... 2\n";
            if (!str) { Exception::throwStandard(ErrorCode::BadArgs); }

            // Parse all functions
            SVGFunctionValuesParser parser(str);
            std::cout << "..... 3\n";

            while (true) {
std::cout << "..... 4, m_transform_count: " << m_transform_count << std::endl;
                if(m_transform_count >= kTransformStackCapacity) {
                    Exception::throwSpecific(kErr_TransformStackOverflow);
                }
                std::cout << "..... 5\n";

                auto status = parser.nextFunction();
                std::cout << "..... 5 1, status: " << status << std::endl;
                if (status < 0) {
                std::cout << "..... 5 2\n";
                    break;
                    // Todo: Exception::throwSpecific(kErr_ParseTransform_InvalidFunctionName);
                }

std::cout << "..... 6\n";
                auto transform = &m_transform_stack[m_transform_count];

                int32_t value_count = parser.extractCSSValues(SVGTransform::kValuesCapacity, transform->m_values);
                if (value_count < 0) {
                    Exception::throwSpecific(kErr_ParseTransform_ValuesParsingFailed);
                }
                std::cout << "..... 7\n";

                transform->m_value_count = value_count;

                auto function_name = parser.functionName();
                if (strcasecmp(function_name, "matrix") == 0) { transform->m_transform_type = SVGTransformType::Matrix; }
                else if (strcasecmp(function_name, "translate") == 0) { transform->m_transform_type = SVGTransformType::Translate; }
                else if (strcasecmp(function_name, "scale") == 0) { transform->m_transform_type = SVGTransformType::Scale; }
                else if (strcasecmp(function_name, "rotate") == 0) { transform->m_transform_type = SVGTransformType::Rotate; }
                else if (strcasecmp(function_name, "skewX") == 0) { transform->m_transform_type = SVGTransformType::SkewX; }
                else if (strcasecmp(function_name, "skewY") == 0) { transform->m_transform_type = SVGTransformType::SkewY; }
                else if (strcasecmp(function_name, "perspective") == 0) { transform->m_transform_type = SVGTransformType::Perspective; }
                else {
                    _m_css_error_count++;
                }

std::cout << "..... 8: _m_css_error_count: " << _m_css_error_count << ", transform->m_transform_type: " << (int)transform->m_transform_type << std::endl;
                m_transform_count++;
            }
        }
        catch (const Exception& e) {
            result = e.code();
        }

std::cout << "..... 9\n";
        return result;
    }


    void SVGTransform::transformGC(GraphicContext& gc) const noexcept {
        switch (m_transform_type) {
            case SVGTransformType::Matrix:
                break;
            case SVGTransformType::Translate:
                break;
            case SVGTransformType::Scale:
                break;
            case SVGTransformType::Rotate:
                if (m_value_count == 1) {
                    gc.rotate(m_values[0].valueAsDouble());
                }
                else if (m_value_count == 3) {
                    gc.rotate(m_values[0].valueAsDouble());
                    gc.translate(m_values[1].valueAsDouble(), m_values[2].valueAsDouble());
                }
                break;
            case SVGTransformType::SkewX:
                break;
            case SVGTransformType::SkewY:
                break;
            case SVGTransformType::Perspective:
                break;
        }
    }


    void SVGPaintStyle::log(std::ostream& os, int32_t indent, const char* label) const {
        Log l(os);
        l.header(label);
        l << "m_attr_opacity: " << m_attr_opacity << Log::endl;
        l << "m_attr_color: " << m_attr_color << Log::endl;
        l << "m_attr_fill: " << m_attr_fill << Log::endl;
        l << "m_attr_fill_rule: " << m_attr_fill_rule << Log::endl;
        l << "m_attr_fill_opacity: " << m_attr_fill_opacity << Log::endl;
        l << "m_attr_stroke: " << m_attr_stroke << Log::endl;
        l << "m_attr_stroke_width: " << m_attr_stroke_width << Log::endl;
        l << "m_attr_stroke_linecap: " << m_attr_stroke_linecap << Log::endl;
        l << "m_attr_stroke_linejoin: " << m_attr_stroke_linejoin << Log::endl;
    }

} // End of namespace
