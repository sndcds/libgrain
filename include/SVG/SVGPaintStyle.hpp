//
// SVGPaintStyle.hpp
//
// Created by Roald Christesen on 03.12.2024
// Copyright (C) 2025 Roald Christesen. All rights reserved.
//
// This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainSVGPaintStyle_hpp
#define GrainSVGPaintStyle_hpp

#include "SVG/SVG.hpp"
#include "Type/Object.hpp"
#include "String/String.hpp"
#include "Color/RGBA.hpp"
#include "CSS/CSS.hpp"
#include "Type/Fix.hpp"
#include "Core/Log.hpp"

#include "Extern/tinyxml2.h"


namespace Grain {


    class GraphicContext;
    class SVGElement;


    typedef struct {
        const char* m_key;
        int64_t m_value;
    } SVGNumericAttrKeyValue;


    class SVGAttr {

    public:
        enum AttrType {
            Undefined = -1,
            Numeric = 0,
            Color
        };

        enum class SetCommand {
            DoNothing = 0,
            SetByInitialValue,
            SetToCurrentColor,
            SetByInheritance,
            SetToURL,
        };

        enum {
            kFlag_HasValue    = 0x1 << 0,
            kFlag_MustUpdate  = 0x1 << 1,
            kFlag_CanInitial  = 0x1 << 2,
            kFlag_CanUnset    = 0x1 << 3,
            kFlag_CanInherit  = 0x1 << 4,
            kFlag_CanBeNone   = 0x1 << 5,

            kFlag_SetIsNone   = 0x1 << 6,
            kFlag_IsRoot      = 0x1 << 7,

            kFlags_Default = kFlag_CanInitial | kFlag_CanUnset | kFlag_CanInherit | kFlag_CanBeNone
        };


    public:
        AttrType m_attr_type = AttrType::Undefined;

        bool m_has_value = false;       ///< Indicates that the attribute is set with a value.
        bool m_must_update = true;      ///< Indicates that the attribute must be updated.
        bool m_is_none = false;         ///< Indicates that the attribute is set to `none`.

        bool m_can_initial = false;
        bool m_can_unset = false;
        bool m_can_inherit = false;
        bool m_can_be_none = false;

        SetCommand m_set_command = SetCommand::DoNothing;
        String m_initial_value;

    public:
        AttrType type() const noexcept { return m_attr_type; }
        bool isNumericType() const noexcept { return m_attr_type == AttrType::Numeric; }
        bool isColorType() const noexcept { return m_attr_type == AttrType::Color; }

        friend std::ostream& operator << (std::ostream& os, const SVGAttr* o) {
            o == nullptr ? os << "SVGAttr nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const SVGAttr& o) {
            o.log(os, 0, "SVGAttr");
            return os;
        }

        void log(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const {
            Log l(os);
            l.header(label);
            l << "m_has_value: " << m_has_value << l.endl;
            l << "m_must_update: " << m_must_update << l.endl;
            l << "m_can_initial: " << m_can_initial << l.endl;
            l << "m_can_unset: " << m_can_unset << l.endl;
            l << "m_can_inherit: " << m_can_inherit << l.endl;
            l << "m_can_be_none: " << m_can_be_none << l.endl;
        }

        bool hasValue() const noexcept { return m_has_value; }
        bool isNone() const noexcept { return m_is_none; }
        bool mustUpdate() const noexcept { return m_must_update; }
        bool canInitial() const noexcept { return m_can_initial; }
        bool canUnset() const noexcept { return m_can_unset; }
        bool canInherit() const noexcept { return m_can_inherit; }
        bool canBeNone() const noexcept { return m_can_be_none; }

        void initWithFlags(uint32_t attr_flags) noexcept;

        void set(const char* str) noexcept;
    };


    class SVGNumericAttr : public SVGAttr {
    public:
        CSSValue m_css_value;
        Fix m_min = 0;
        Fix m_max = 999999999;
        const SVGNumericAttrKeyValue* m_key_value_table = nullptr;

    public:
        SVGNumericAttr() {
            m_attr_type = AttrType::Numeric;
        }

        friend std::ostream& operator << (std::ostream& os, const SVGNumericAttr* o) {
            o == nullptr ? os << "SVGNumericAttr nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const SVGNumericAttr& o) {
            o.log(os, 0, nullptr);
            return os;
        }

        void log(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const {

            Log l(os);
            l.header(label);

            SVGAttr::log(os, indent, nullptr);

            l << "m_css_value: " << m_css_value.value() << " " << m_css_value.unitName() << l.endl;
            l << "m_min: " << m_min << ", m_max: " << m_max << l.endl;
            if (m_key_value_table != nullptr) {
                l << "key value table:" << l.endl;
                l++;
                int32_t index = 0;
                while (m_key_value_table[index].m_key[0] != '\0') {
                    l << index << ": " << m_key_value_table[index].m_key << ", " << m_key_value_table[index].m_value << l.endl;
                    index++;
                }
            }
        }

        void setKeyValueTable(const SVGNumericAttrKeyValue* table) noexcept { m_key_value_table = table; }

        void setMin(int32_t min) noexcept { m_min = min; }
        void setMax(int32_t max) noexcept { m_max = max; }
        void setMinMax(int32_t min, int32_t max) noexcept { m_min = min; m_max = max; }
        void setMin(double min) noexcept { m_min = min; }
        void setMax(double max) noexcept { m_max = max; }
        void setMinMax(double min, double max) noexcept { m_min = min; m_max = max; }

        bool setValue(const char* str, const CSSValue& initial_value) noexcept;
        void setDouble(int64_t value, CSSUnit unit)  noexcept { m_css_value.setDouble(value, unit); }
        void setDoubleAbsolute(int64_t value)  noexcept { setDouble(value, CSSUnit::Absolute); }
        void setInt32(int32_t value, CSSUnit unit) noexcept { m_css_value.setInt32(value, unit); }
        void setInt32Absolute(int32_t value)  noexcept { setInt32(value, CSSUnit::Absolute); }
        const CSSValue& value() const noexcept { return m_css_value; }

        const double valueAsDouble() const noexcept { return m_css_value.valueAsDouble(); }
        const int32_t valueAsInt32() const noexcept { return m_css_value.valueAsInt32(); }

        void undef() noexcept { m_css_value.undef(); }
    };


    class SVGColorAttr : public SVGAttr {

        enum class DrawMode {
            Color = 0,
            Gradient,
        };

    public:
        RGBA m_color = { 0.0f, 0.0f, 0.0f, 1.0f };
        bool m_use_current_color = false;
        DrawMode m_draw_mode = DrawMode::Color;
        SVGPaintServer* m_paint_server = nullptr;
        String m_raw;

    public:
        SVGColorAttr() {
            m_attr_type = AttrType::Color;
        }

        friend std::ostream& operator << (std::ostream& os, const SVGColorAttr* o) {
            o == nullptr ? os << "SVGColorAttr nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const SVGColorAttr& o) {
            o.log(os, 0, "SVGColorAttr");
            return os;
        }

        void log(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const {
            Log l(os);
            l.header(label);
            SVGAttr::log(os, indent, nullptr);
            l << "m_color: " << m_color << l.endl;
            l << "m_use_current_color: " << m_use_current_color << l.endl;
            l << "m_set_command: " << (int32_t)m_set_command << l.endl;
            l << "m_raw: " << m_raw << l.endl;
        }

        bool setColor(const char* str, const RGBA& initial_color) noexcept;
        void setColor(const RGBA& color) noexcept {
            m_color = color;
            m_has_value = true;
        }
        const RGBA& color() const noexcept { return m_color; }
    };


    class SVGTransform {

    public:
        enum {
            kValuesCapacity = 6
        };

    public:
        SVGTransformType m_transform_type;
        CSSValue m_values[kValuesCapacity];
        int32_t m_value_count;

    public:
        void transformGC(GraphicContext& gc) const noexcept;
    };


    class SVGPaintStyle : protected Object {

        friend class SVGElement;
        friend class SVGPaintElement;
        friend class SVGRectElement;
        friend class SVGCircleElement;

    public:
        enum class AttrID {
            Color = 0,
            Fill,
            FillRule,
            FillOpacity,
            Stroke,
            StrokeWidth,
            StrokeLinecap,
            StrokeLinejoin,
            StrokeMiterlimit,
            StrokeOpacity,
        };

        enum {
            kTransformStackCapacity = 8
        };

        enum {
            kErr_ParseTransform_ToManyValues = 0,
            kErr_ParseTransform_ValueExtractionFailed,
            kErr_ParseTransform_InvalidFunctionName,
            kErr_ParseTransform_ValuesParsingFailed,
            kErr_TransformStackOverflow,
        };

    protected:

        SVGElement* m_svg_element_ptr = nullptr;

        SVGNumericAttr m_attr_opacity;

        SVGColorAttr m_attr_color;

        SVGColorAttr m_attr_fill;
        SVGNumericAttr m_attr_fill_rule;
        SVGNumericAttr m_attr_fill_opacity;

        SVGColorAttr m_attr_stroke;
        SVGNumericAttr m_attr_stroke_width;
        SVGNumericAttr m_attr_stroke_linecap;
        SVGNumericAttr m_attr_stroke_linejoin;
        SVGNumericAttr m_attr_stroke_miterlimit;
        SVGNumericAttr m_attr_stroke_opacity;


        // Transform.

        SVGTransform m_transform_stack[kTransformStackCapacity];
        int32_t m_transform_count = 0;


        //

        bool m_does_fill = false;               ///< Indicates if style draws fills.
        bool m_has_fill_opacity = false;        ///< Indicates if style has fill-opacity.
        bool m_does_stroke = false;             ///< Indicates if style draws stroke.
        bool m_has_stroke_linecap = false;      ///< Indicates if style has stroke-linecap.
        bool m_has_stroke_linejoin = false;     ///< Indicates if style has stroke-linejoin.
        bool m_has_stroke_miterlimit = false;   ///< Indicates if style has stroke-miterlimit.
        bool m_has_stroke_opacity = false;      ///< Indicates if style has stroke-opacity.


        int32_t _m_css_error_count = 0;         ///< Number of errors in CSS parsing.
        int32_t _m_css_warning_count = 0;       ///< Number of warnings in CSS parsing.
        int32_t _m_fatal_error_count = 0;       ///< Number of fatal errors, should never happen.


        String m_stroke_dasharray;              // Don't inherit, default: none
        String m_stroke_dashoffset;             // Don't inherit, default: 0


        static const SVGNumericAttrKeyValue _g_fill_rule_table[];
        static const SVGNumericAttrKeyValue _g_stroke_linecap_table[];
        static const SVGNumericAttrKeyValue _g_stroke_linejoin_table[];

    public:
        SVGPaintStyle() {
        }

        ~SVGPaintStyle() {
        }

        const char* className() const noexcept override { return "SVGPaintStyle"; }


        friend std::ostream& operator << (std::ostream& os, const SVGPaintStyle* o) {
            o == nullptr ? os << "SVGPaintStyle nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const SVGPaintStyle& o) {
            o.log(os, 0, nullptr);
            return os;
        }

        virtual void log(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const;


        SVGAttr* attrByID(AttrID attr_id) noexcept {
            switch (attr_id) {
                case AttrID::Color: return &m_attr_color;
                case AttrID::Fill: return &m_attr_fill;
                case AttrID::FillRule: return &m_attr_fill_rule;
                case AttrID::FillOpacity: return &m_attr_fill_opacity;
                case AttrID::Stroke: return &m_attr_stroke;
                case AttrID::StrokeWidth: return &m_attr_stroke_width;
                case AttrID::StrokeLinecap: return &m_attr_stroke_linecap;
                case AttrID::StrokeLinejoin: return &m_attr_stroke_linejoin;
                case AttrID::StrokeMiterlimit: return &m_attr_stroke_miterlimit;
                case AttrID::StrokeOpacity: return &m_attr_stroke_opacity;
            }
            return nullptr;
        }

        SVGNumericAttr* numericAttrByID(AttrID attr_id) noexcept {
            switch (attr_id) {
                case AttrID::FillRule: return &m_attr_fill_rule;
                case AttrID::FillOpacity: return &m_attr_fill_opacity;
                case AttrID::StrokeWidth: return &m_attr_stroke_width;
                case AttrID::StrokeLinecap: return &m_attr_stroke_linecap;
                case AttrID::StrokeLinejoin: return &m_attr_stroke_linejoin;
                case AttrID::StrokeMiterlimit: return &m_attr_stroke_miterlimit;
                case AttrID::StrokeOpacity: return &m_attr_stroke_opacity;
                default: return nullptr;
            }
        }

        SVGColorAttr* colorAttrByID(AttrID attr_id) noexcept {
            switch (attr_id) {
                case AttrID::Color: return &m_attr_color;
                case AttrID::Fill: return &m_attr_fill;
                case AttrID::Stroke: return &m_attr_stroke;
                default: return nullptr;
            }
        }

        void updateAllAttr() noexcept {

            updatColorAttrWithID(AttrID::Color);
            updatColorAttrWithID(AttrID::Fill);
            updateNumericAttrWithID(AttrID::FillRule);
            updateNumericAttrWithID(AttrID::FillOpacity);
            updatColorAttrWithID(AttrID::Stroke);
            updateNumericAttrWithID(AttrID::StrokeWidth);
            updateNumericAttrWithID(AttrID::StrokeLinecap);
            updateNumericAttrWithID(AttrID::StrokeLinejoin);
            updateNumericAttrWithID(AttrID::StrokeMiterlimit);
            updateNumericAttrWithID(AttrID::StrokeOpacity);

            m_does_fill = m_attr_fill.hasValue() && !m_attr_fill.isNone();
            m_does_stroke = m_attr_stroke.hasValue() && !m_attr_stroke.isNone() && m_attr_stroke_width.valueAsDouble() > FLT_EPSILON;
        }

        void updateNumericAttrWithID(AttrID attr_id) noexcept {
            updateNumericAttr(attr_id, nullptr);
        }

        void updatColorAttrWithID(AttrID attr_id) noexcept {
            updateColorAttr(attr_id, nullptr);
        }

        bool doesFill() const noexcept { return m_does_fill; }
        bool doesStroke() const noexcept { return m_does_stroke; }

        const RGBA& color() const noexcept { return m_attr_color.m_color; }
        const RGBA& fillColor() const noexcept { return m_attr_fill.m_color; }
        const RGBA& strokeColor() const noexcept { return m_attr_stroke.m_color; }

        void setGCSettings(GraphicContext& gc) const noexcept;

        void setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept;;
        void setDefault() noexcept;

        ErrorCode parseTransform(const char* str) noexcept;


        void updateNumericAttr(AttrID attr_id, SVGNumericAttr* out_attr) noexcept;
        void updateColorAttr(AttrID attr_id, SVGColorAttr* out_attr) noexcept;
    };

} // End of namespace

#endif // GrainSVGPaintStyle_hpp