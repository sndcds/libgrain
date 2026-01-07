//
//  SVGPaintStyle.cpp
//
//  Created by Roald Christesen on 03.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#include "SVG/SVGPaintStyle.hpp"
#include "SVG/SVG.hpp"
#include "SVG/SVGElement.hpp"
#include "CSS/CSS.hpp"
#include "CSS/CSSColor.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Core/Log.hpp"


namespace Grain {

    const SVGNumericAttrKeyValue SVGPaintStyle::g_fill_rule_table_[] = {
        { "nonzero", (int64_t)FillWindingRule::NoneZero },
        { "evenodd", (int64_t)FillWindingRule::EvenOdd },
        { "", -1 }
    };

    const SVGNumericAttrKeyValue SVGPaintStyle::g_stroke_linecap_table_[] = {
        { "butt", (int64_t)StrokeCapStyle::Butt },
        { "round", (int64_t)StrokeCapStyle::Round },
        { "square", (int64_t)StrokeCapStyle::Square },
        { "", -1 }
    };

    const SVGNumericAttrKeyValue SVGPaintStyle::g_stroke_linejoin_table_[] = {
        { "miter", (int64_t)StrokeJoinStyle::Miter },
        { "round", (int64_t)StrokeJoinStyle::Round },
        { "bevel", (int64_t)StrokeJoinStyle::Bevel },
        { "", -1 }
    };


    void SVGAttr::initWithFlags(uint32_t attr_flags) noexcept {
        can_initial_ = (attr_flags & kFlag_CanInitial) != 0;
        can_unset_ = (attr_flags & kFlag_CanUnset) != 0;
        can_inherit_ = (attr_flags & kFlag_CanInherit) != 0;
        can_be_none_ = (attr_flags & kFlag_CanBeNone) != 0;
    };


    void SVGAttr::set(const char* str) noexcept {
        has_value_ = false;
        must_update_ = true;
        is_none_ = false;
        set_command_ = SetCommand::DoNothing;

        if (str == nullptr) {
            return;
        }

        if (canBeNone() && (strcasecmp(str, "none") == 0 || strcasecmp(str, "transparent") == 0)) {
            if (canInherit()) {
                set_command_ = SetCommand::SetByInheritance;
            }
            has_value_ = true;
            is_none_ = true;
        }

        if (canInherit() && strcasecmp(str, "inherit") == 0) {
            set_command_ = SetCommand::SetByInheritance;
        }

        if (canInitial() && strcasecmp(str, "initial") == 0) {
            set_command_ = SetCommand::SetByInitialValue;
        }

        if (canUnset() && strcasecmp(str, "unset") == 0) {
            if (canInherit()) {
                // Equivalent to inherit.
                set_command_ = SetCommand::SetByInheritance;
            }
            else {
                // Equivalent to initial.
                set_command_ = SetCommand::SetByInitialValue;
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
        use_current_color_ = false;

        str = String::firstNonWhiteSpaceCharPtr(str);

        SVGAttr::set(str);

        if (set_command_ == SetCommand::SetByInitialValue) {
            color_ = initial_color;
            has_value_ = true;
            return true;
        }

        if (set_command_ == SetCommand::SetByInheritance) {
            // Will be handled later.
            return true;
        }

        if (strcasecmp(str, "currentColor") == 0) {
            set_command_ = SetCommand::SetToCurrentColor;
            has_value_ = true;
            return true;
        }

        if (strncasecmp(str, "url(", 4) == 0) {
            int64_t n = raw_.setByFramedContent(str, '(', ')');
            if (n >= 0) {
                draw_mode_ = DrawMode::Gradient;
                set_command_ = SetCommand::SetToURL;
                has_value_ = true;
                return true;
            }
        }

        auto err = CSSColor::parseColorToRGBA(str, color_);
        if (Error::isError(err)) {
            return false;
        }
        else {
            has_value_ = true;
            return true;
        }
    }


    bool SVGNumericAttr::setValue(const char* str, const CSSValue& initial_value) noexcept {
        str = String::firstNonWhiteSpaceCharPtr(str);

        SVGAttr::set(str);

        if (set_command_ == SetCommand::SetByInitialValue) {
            css_value_ = initial_value;
            has_value_ = true;
            return true;
        }

        if (set_command_ == SetCommand::SetByInheritance) {
            // Will be handled later.
            return true;
        }

        // Key/Value table.

        if (key_value_table_ != nullptr) {
            int32_t index = 0;
            while (key_value_table_[index].key_[0] != '\0') {
                if (strcasecmp(str, key_value_table_[index].key_) == 0) {
                    has_value_ = true;
                    css_value_.setInt32Absolute((int32_t)key_value_table_[index].value_);
                    return true;
                }
                index++;
            }
        }

        auto err = CSS::extractCSSValueFromStr(str, css_value_, nullptr);
        if (Error::isError(err)) {
            return false;
        }
        else {
            has_value_ = true;
            return true;
        }
    }


    /**
     *  @brief:
     */
    void SVGPaintStyle::updateNumericAttr(AttrID attr_id, SVGNumericAttr* out_attr) noexcept {
        auto attr = numericAttrByID(attr_id);
        if (attr == nullptr) {
            fatal_error_count_++;
            return;
        }

        if (attr->mustUpdate()) {
            if (attr->hasValue() == false && attr->canInherit() == true) {
                auto parent = svg_element_ptr_->parent();
                if (parent != nullptr) {
                    auto parent_paint_style = (SVGPaintStyle*)parent->paintStyle();
                    if (parent_paint_style != nullptr) {
                        parent_paint_style->updateNumericAttr(attr_id, attr);
                    }
                    else {
                        fatal_error_count_++;
                    }
                }
            }

            attr->must_update_ = false;
        }

        if (out_attr != nullptr) {
            out_attr->css_value_ = attr->css_value_;
            out_attr->has_value_ = attr->has_value_;
            out_attr->is_none_ = attr->is_none_;
        }
    }


    /**
     *  @brief:
     */
    void SVGPaintStyle::updateColorAttr(AttrID attr_id, SVGColorAttr* out_attr) noexcept {
        auto attr = colorAttrByID(attr_id);
        if (attr == nullptr) {
            fatal_error_count_++;
            return;
        }

        if (attr->mustUpdate() == true) {

            if (attr->hasValue() == false && attr->canInherit() == true) {
                auto parent = svg_element_ptr_->parent();
                if (parent != nullptr) {
                    auto parent_paint_style = (SVGPaintStyle*)parent->paintStyle();
                    if (parent_paint_style != nullptr) {
                        parent_paint_style->updateColorAttr(attr_id, attr);
                    }
                    else {
                        fatal_error_count_++;
                    }
                }
            }


            attr->must_update_ = false;
        }

        if (out_attr != nullptr) {
            out_attr->color_ = attr->color_;
            out_attr->use_current_color_ = attr->use_current_color_;
            out_attr->has_value_ = attr->has_value_;
            out_attr->is_none_ = attr->is_none_;
        }
    }


    void SVGPaintStyle::setGCSettings(GraphicContext& gc) const noexcept {
        if (transform_count_ > 0) {
            for (int32_t i = 0; i < transform_count_; i++) {
                transform_stack_[i].transformGC(gc);
            }
        }

        if (attr_opacity_.hasValue()) {
            gc.setAlpha(attr_opacity_.valueAsDouble());
        }

        if (attr_fill_.hasValue()) {

            if (attr_fill_.use_current_color_) {
                gc.setFillRGB(attr_color_.color_);
            }
            else {
                gc.setFillRGB(attr_fill_.color_);
            }
        }

        if (attr_stroke_.hasValue()) {
            if (attr_stroke_.use_current_color_) {
                gc.setStrokeRGB(attr_color_.color_);
            }
            else {
                gc.setStrokeRGB(attr_stroke_.color_);
            }
        }

        if (attr_stroke_width_.hasValue()) {
            gc.setStrokeWidth(attr_stroke_width_.css_value_.valueAsDouble());  // TODO: Handle unit.
        }

        if (attr_stroke_linecap_.hasValue()) {
            gc.setStrokeCapStyle((StrokeCapStyle)attr_stroke_linecap_.css_value_.valueAsInt32());
        }

        if (attr_stroke_linejoin_.hasValue()) {
            gc.setStrokeJoinStyle((StrokeJoinStyle)attr_stroke_linejoin_.css_value_.valueAsInt32());
        }

        if (attr_stroke_miterlimit_.hasValue()) {
            gc.setStrokeMiterLimit(attr_stroke_miterlimit_.css_value_.valueAsDouble());
        }
    }


    /**
     *  @brief Set all paint style properties with values from `xml_element`.
     */
    void SVGPaintStyle::setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept {
        if (xml_element != nullptr) {
            RGBA default_color = { 0.0f, 0.0f, 0.0f, 1.0f };

            // Iterate over all attributes of the element
            const tinyxml2::XMLAttribute* attribute = xml_element->FirstAttribute();
            while (attribute != nullptr) {
                CSSValue default_css_value; // Is undefined by default
                auto name = attribute->Name();
                auto value = attribute->Value();

                bool set_result = false;

                if (strcasecmp(name, "opacity") == 0) {
                    set_result = attr_opacity_.setValue(value, default_css_value);
                }
                else if (strcasecmp(name, "color") == 0) {
                    set_result = attr_color_.setColor(value, default_color);
                }
                else if (strcasecmp(name, "fill") == 0) {
                    set_result = attr_fill_.setColor(value, default_color);
                }
                else if (strcasecmp(name, "fill-rule") == 0) {
                    set_result = attr_fill_rule_.setValue(xml_element->Attribute("fill-rule"), default_css_value);
                }
                else if (strcasecmp(name, "fill-opacity") == 0) {
                    set_result = attr_fill_opacity_.setValue(xml_element->Attribute("fill-rule"), default_css_value);
                }
                else if (strcasecmp(name, "stroke") == 0) {
                    set_result = attr_stroke_.setColor(value, default_color);
                }
                else if (strcasecmp(name, "stroke-width") == 0) {
                    set_result = attr_stroke_width_.setValue(value, default_css_value);
                }
                else if (strcasecmp(name, "stroke-linecap") == 0) {
                    set_result = attr_stroke_linecap_.setValue(value, default_css_value);
                }
                else if (strcasecmp(name, "stroke-linejoin") == 0) {
                    set_result = attr_stroke_linecap_.setValue(value, default_css_value);
                }
                else if (strcasecmp(name, "stroke-miterlimit") == 0) {
                    set_result = attr_stroke_miterlimit_.setValue(value, default_css_value);
                }
                else if (strcasecmp(name, "stroke-opacity") == 0) {
                    set_result = attr_stroke_opacity_.setValue(value, default_css_value);
                }
                else if (strcasecmp(name, "transform") == 0) {
                    parseTransform(value);
                }

                if (set_result != true) {
                    // TODO: Warning or Error Message!!!!!
                }

                attribute = attribute->Next();
            }

            /* TODO: !!!!
            m_stroke_dasharray = SVG::_validStr(e->Attribute("stroke-dasharray"));
            m_stroke_dashoffset = SVG::_validStr(e->Attribute("stroke-dashoffset"));
             */
        }
    }


    void SVGPaintStyle::setDefault() noexcept {
        attr_opacity_.initWithFlags(SVGAttr::kFlags_Default);
        attr_opacity_.setDoubleAbsolute(1.0);
        attr_opacity_.setMinMax(0, 1);

        // Color
        attr_color_.initWithFlags(SVGAttr::kFlags_Default);
        attr_color_.setColor(RGBA(0.0f, 0.0f, 0.0f, 1.0f));

        // Fill
        attr_fill_.initWithFlags(SVGAttr::kFlags_Default);
        attr_fill_.setColor(RGBA(0.0f, 0.0f, 0.0f, 1.0f));

        attr_fill_rule_.setKeyValueTable(g_fill_rule_table_);
        attr_fill_rule_.initWithFlags(SVGAttr::kFlags_Default);
        attr_fill_rule_.setInt32Absolute((int32_t)FillWindingRule::NoneZero);

        attr_fill_opacity_.initWithFlags(SVGAttr::kFlags_Default);
        attr_fill_opacity_.setDoubleAbsolute(1.0);
        attr_fill_opacity_.setMinMax(0, 1);

        // Stroke
        attr_stroke_.initWithFlags(SVGAttr::kFlags_Default);

        attr_stroke_width_.initWithFlags(SVGAttr::kFlags_Default);

        attr_stroke_linecap_.setKeyValueTable(g_stroke_linecap_table_);
        attr_stroke_linecap_.initWithFlags(SVGAttr::kFlags_Default);
        attr_stroke_linecap_.setInt32Absolute((int32_t)StrokeCapStyle::Butt);

        attr_stroke_linejoin_.setKeyValueTable(g_stroke_linejoin_table_);
        attr_stroke_linejoin_.initWithFlags(SVGAttr::kFlags_Default);
        attr_stroke_linejoin_.setInt32Absolute((int32_t)StrokeJoinStyle::Miter);

        attr_stroke_miterlimit_.initWithFlags(SVGAttr::kFlags_Default);
        attr_stroke_miterlimit_.setMinMax(1, 10);

        attr_stroke_opacity_.initWithFlags(SVGAttr::kFlags_Default);
        attr_stroke_opacity_.setDoubleAbsolute(1.0);
        attr_stroke_opacity_.setMinMax(0, 1);

        has_fill_opacity_ = false;
        has_stroke_linecap_ = false;
        has_stroke_linejoin_ = false;
        has_stroke_miterlimit_ = false;
        has_stroke_opacity_ = false;

        does_fill_ = false;
        does_stroke_ = false;

        // TODO: stroke-dasharray none
        // TODO: stroke-dashoffset 0
    }


    ErrorCode SVGPaintStyle::parseTransform(const char* str) noexcept {
        auto result = ErrorCode::None;

        try {
            if (!str) { Exception::throwStandard(ErrorCode::BadArgs); }

            // Parse all functions
            SVGFunctionValuesParser parser(str);

            while (true) {
                if(transform_count_ >= kTransformStackCapacity) {
                    Exception::throwSpecific(kErr_TransformStackOverflow);
                }

                auto status = parser.nextFunction();
                if (status < 0) {
                    break;
                    // TODO: Exception::throwSpecific(kErr_ParseTransform_InvalidFunctionName);
                }

                auto transform = &transform_stack_[transform_count_];

                int32_t value_count = parser.extractCSSValues(SVGTransform::kValuesCapacity, transform->values_);
                if (value_count < 0) {
                    Exception::throwSpecific(kErr_ParseTransform_ValuesParsingFailed);
                }

                transform->value_count_ = value_count;

                auto function_name = parser.functionName();
                if (strcasecmp(function_name, "matrix") == 0) { transform->transform_type_ = SVGTransformType::Matrix; }
                else if (strcasecmp(function_name, "translate") == 0) { transform->transform_type_ = SVGTransformType::Translate; }
                else if (strcasecmp(function_name, "scale") == 0) { transform->transform_type_ = SVGTransformType::Scale; }
                else if (strcasecmp(function_name, "rotate") == 0) { transform->transform_type_ = SVGTransformType::Rotate; }
                else if (strcasecmp(function_name, "skewX") == 0) { transform->transform_type_ = SVGTransformType::SkewX; }
                else if (strcasecmp(function_name, "skewY") == 0) { transform->transform_type_ = SVGTransformType::SkewY; }
                else if (strcasecmp(function_name, "perspective") == 0) { transform->transform_type_ = SVGTransformType::Perspective; }
                else {
                    css_error_count_++;
                }

                transform_count_++;
            }
        }
        catch (const Exception& e) {
            result = e.code();
        }

        return result;
    }


    void SVGTransform::transformGC(GraphicContext& gc) const noexcept {
        switch (transform_type_) {
            case SVGTransformType::Matrix: {
                if (value_count_ == 6) {
                    Mat3d m;
                    m.setSVGTransform(
                        values_[0].valueAsDouble(), values_[1].valueAsDouble(), values_[2].valueAsDouble(),
                        values_[3].valueAsDouble(), values_[4].valueAsDouble(), values_[5].valueAsDouble());
                    gc.affineTransform(m);
                }
                break;
            }
            case SVGTransformType::Translate: {
                if (value_count_ == 1) {
                    gc.translateX(values_[0].valueAsDouble());
                }
                else if (value_count_ == 2) {
                    gc.translate(values_[0].valueAsDouble(), values_[1].valueAsDouble());
                }
                break;
            }
            case SVGTransformType::Scale: {
                if (value_count_ == 1) {
                    gc.scale(values_[0].valueAsDouble());
                }
                else if (value_count_ == 2) {
                    gc.scale(values_[0].valueAsDouble(), values_[1].valueAsDouble());
                }
                break;
            }
            case SVGTransformType::Rotate: {
                if (value_count_ == 1) {
                    gc.rotate(values_[0].valueAsDouble());
                }
                else if (value_count_ == 3) {
                    Vec2d pivot = { values_[1].valueAsDouble(), values_[2].valueAsDouble() };
                    gc.rotateAroundPivot(pivot, values_[0].valueAsDouble());
                }
                break;
            }
            case SVGTransformType::SkewX: {
                // TODO: Implement!
                break;
            }
            case SVGTransformType::SkewY: {
                // TODO: Implement!
                break;
            }
            case SVGTransformType::Perspective: {
                // TODO: Implement!
                break;
            }
        }
    }


    void SVGPaintStyle::log(std::ostream& os, int32_t indent, const char* label) const {
        Log l(os);
        l.header(label);
        l << "attr_opacity_: " << attr_opacity_ << Log::endl;
        l << "attr_color_: " << attr_color_ << Log::endl;
        l << "attr_fill_: " << attr_fill_ << Log::endl;
        l << "attr_fill_rule_: " << attr_fill_rule_ << Log::endl;
        l << "attr_fill_opacity_: " << attr_fill_opacity_ << Log::endl;
        l << "attr_stroke_: " << attr_stroke_ << Log::endl;
        l << "attr_stroke_width_: " << attr_stroke_width_ << Log::endl;
        l << "attr_stroke_linecap_: " << attr_stroke_linecap_ << Log::endl;
        l << "attr_stroke_linejoin_: " << attr_stroke_linejoin_ << Log::endl;
    }

} // End of namespace
