//
//  SVGPaintStyle.hpp
//
//  Created by Roald Christesen on 03.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
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
        const char* key_;
        int64_t value_;
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
        AttrType attr_type_ = AttrType::Undefined;

        bool has_value_ = false;       ///< Indicates that the attribute is set with a value.
        bool must_update_ = true;      ///< Indicates that the attribute must be updated.
        bool is_none_ = false;         ///< Indicates that the attribute is set to `none`.
        bool can_initial_ = false;
        bool can_unset_ = false;
        bool can_inherit_ = false;
        bool can_be_none_ = false;

        SetCommand set_command_ = SetCommand::DoNothing;
        String initial_value_;

    public:
        AttrType type() const noexcept { return attr_type_; }
        bool isNumericType() const noexcept { return attr_type_ == AttrType::Numeric; }
        bool isColorType() const noexcept { return attr_type_ == AttrType::Color; }

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
            l << "m_has_value: " << has_value_ << l.endl;
            l << "m_must_update: " << must_update_ << l.endl;
            l << "m_can_initial: " << can_initial_ << l.endl;
            l << "m_can_unset: " << can_unset_ << l.endl;
            l << "m_can_inherit: " << can_inherit_ << l.endl;
            l << "m_can_be_none: " << can_be_none_ << l.endl;
        }

        bool hasValue() const noexcept { return has_value_; }
        bool isNone() const noexcept { return is_none_; }
        bool mustUpdate() const noexcept { return must_update_; }
        bool canInitial() const noexcept { return can_initial_; }
        bool canUnset() const noexcept { return can_unset_; }
        bool canInherit() const noexcept { return can_inherit_; }
        bool canBeNone() const noexcept { return can_be_none_; }

        void initWithFlags(uint32_t attr_flags) noexcept;

        void set(const char* str) noexcept;
    };


    class SVGNumericAttr : public SVGAttr {
    public:
        CSSValue css_value_;
        Fix min_ = 0;
        Fix max_ = 999999999;
        const SVGNumericAttrKeyValue* key_value_table_ = nullptr;

    public:
        SVGNumericAttr() {
            attr_type_ = AttrType::Numeric;
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

            l << "m_css_value: " << css_value_.value() << " " << css_value_.unitName() << l.endl;
            l << "m_min: " << min_ << ", m_max: " << max_ << l.endl;
            if (key_value_table_ != nullptr) {
                l << "key value table:" << l.endl;
                l++;
                int32_t index = 0;
                while (key_value_table_[index].key_[0] != '\0') {
                    l << index << ": " << key_value_table_[index].key_ << ", " << key_value_table_[index].value_ << l.endl;
                    index++;
                }
            }
        }

        void setKeyValueTable(const SVGNumericAttrKeyValue* table) noexcept { key_value_table_ = table; }

        void setMin(int32_t min) noexcept { min_ = min; }
        void setMax(int32_t max) noexcept { max_ = max; }
        void setMinMax(int32_t min, int32_t max) noexcept { min_ = min; max_ = max; }
        void setMin(double min) noexcept { min_ = min; }
        void setMax(double max) noexcept { max_ = max; }
        void setMinMax(double min, double max) noexcept { min_ = min; max_ = max; }

        bool setValue(const char* str, const CSSValue& initial_value) noexcept;
        void setDouble(int64_t value, CSSUnit unit)  noexcept { css_value_.setDouble(value, unit); }
        void setDoubleAbsolute(int64_t value)  noexcept { setDouble(value, CSSUnit::Absolute); }
        void setInt32(int32_t value, CSSUnit unit) noexcept { css_value_.setInt32(value, unit); }
        void setInt32Absolute(int32_t value)  noexcept { setInt32(value, CSSUnit::Absolute); }
        const CSSValue& value() const noexcept { return css_value_; }

        const double valueAsDouble() const noexcept { return css_value_.valueAsDouble(); }
        const int32_t valueAsInt32() const noexcept { return css_value_.valueAsInt32(); }

        void undef() noexcept { css_value_.undef(); }
    };


    class SVGColorAttr : public SVGAttr {

        enum class DrawMode {
            Color = 0,
            Gradient,
        };

    public:
        RGBA color_ = { 0.0f, 0.0f, 0.0f, 1.0f };
        bool use_current_color_ = false;
        DrawMode draw_mode_ = DrawMode::Color;
        SVGPaintServer* paint_server_ = nullptr;
        String raw_;

    public:
        SVGColorAttr() {
            attr_type_ = AttrType::Color;
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
            l << "m_color: " << color_ << l.endl;
            l << "m_use_current_color: " << use_current_color_ << l.endl;
            l << "m_set_command: " << (int32_t)set_command_ << l.endl;
            l << "m_raw: " << raw_ << l.endl;
        }

        bool setColor(const char* str, const RGBA& initial_color) noexcept;
        void setColor(const RGBA& color) noexcept {
            color_ = color;
            has_value_ = true;
        }
        const RGBA& color() const noexcept { return color_; }
    };


    class SVGTransform {
    public:
        enum {
            kValuesCapacity = 6
        };

    public:
        SVGTransformType transform_type_;
        CSSValue values_[kValuesCapacity];
        int32_t value_count_;

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
        SVGElement* svg_element_ptr_ = nullptr;
        SVGNumericAttr attr_opacity_;
        SVGColorAttr attr_color_;
        SVGColorAttr attr_fill_;
        SVGNumericAttr attr_fill_rule_;
        SVGNumericAttr attr_fill_opacity_;
        SVGColorAttr attr_stroke_;
        SVGNumericAttr attr_stroke_width_;
        SVGNumericAttr attr_stroke_linecap_;
        SVGNumericAttr attr_stroke_linejoin_;
        SVGNumericAttr attr_stroke_miterlimit_;
        SVGNumericAttr attr_stroke_opacity_;

        // Transform
        SVGTransform transform_stack_[kTransformStackCapacity]{};
        int32_t transform_count_ = 0;

        //
        bool does_fill_ = false;               ///< Indicates if style draws fills
        bool has_fill_opacity_ = false;        ///< Indicates if style has fill-opacity
        bool does_stroke_ = false;             ///< Indicates if style draws stroke
        bool has_stroke_linecap_ = false;      ///< Indicates if style has stroke-linecap
        bool has_stroke_linejoin_ = false;     ///< Indicates if style has stroke-linejoin
        bool has_stroke_miterlimit_ = false;   ///< Indicates if style has stroke-miterlimit
        bool has_stroke_opacity_ = false;      ///< Indicates if style has stroke-opacity

        int32_t css_error_count_ = 0;          ///< Number of errors in CSS parsing
        int32_t css_warning_count_ = 0;        ///< Number of warnings in CSS parsing
        int32_t fatal_error_count_ = 0;        ///< Number of fatal errors, should never happen

        String stroke_dasharray_;              // Don't inherit, default: none
        String stroke_dashoffset_;             // Don't inherit, default: 0

        static const SVGNumericAttrKeyValue g_fill_rule_table_[];
        static const SVGNumericAttrKeyValue g_stroke_linecap_table_[];
        static const SVGNumericAttrKeyValue g_stroke_linejoin_table_[];

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
                case AttrID::Color: return &attr_color_;
                case AttrID::Fill: return &attr_fill_;
                case AttrID::FillRule: return &attr_fill_rule_;
                case AttrID::FillOpacity: return &attr_fill_opacity_;
                case AttrID::Stroke: return &attr_stroke_;
                case AttrID::StrokeWidth: return &attr_stroke_width_;
                case AttrID::StrokeLinecap: return &attr_stroke_linecap_;
                case AttrID::StrokeLinejoin: return &attr_stroke_linejoin_;
                case AttrID::StrokeMiterlimit: return &attr_stroke_miterlimit_;
                case AttrID::StrokeOpacity: return &attr_stroke_opacity_;
            }
            return nullptr;
        }

        SVGNumericAttr* numericAttrByID(AttrID attr_id) noexcept {
            switch (attr_id) {
                case AttrID::FillRule: return &attr_fill_rule_;
                case AttrID::FillOpacity: return &attr_fill_opacity_;
                case AttrID::StrokeWidth: return &attr_stroke_width_;
                case AttrID::StrokeLinecap: return &attr_stroke_linecap_;
                case AttrID::StrokeLinejoin: return &attr_stroke_linejoin_;
                case AttrID::StrokeMiterlimit: return &attr_stroke_miterlimit_;
                case AttrID::StrokeOpacity: return &attr_stroke_opacity_;
                default: return nullptr;
            }
        }

        SVGColorAttr* colorAttrByID(AttrID attr_id) noexcept {
            switch (attr_id) {
                case AttrID::Color: return &attr_color_;
                case AttrID::Fill: return &attr_fill_;
                case AttrID::Stroke: return &attr_stroke_;
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

            does_fill_ = attr_fill_.hasValue() && !attr_fill_.isNone();
            does_stroke_ = attr_stroke_.hasValue() && !attr_stroke_.isNone() && attr_stroke_width_.valueAsDouble() > FLT_EPSILON;
        }

        void updateNumericAttrWithID(AttrID attr_id) noexcept {
            updateNumericAttr(attr_id, nullptr);
        }

        void updatColorAttrWithID(AttrID attr_id) noexcept {
            updateColorAttr(attr_id, nullptr);
        }

        bool doesFill() const noexcept { return does_fill_; }
        bool doesStroke() const noexcept { return does_stroke_; }

        const RGBA& color() const noexcept { return attr_color_.color_; }
        const RGBA& fillColor() const noexcept { return attr_fill_.color_; }
        const RGBA& strokeColor() const noexcept { return attr_stroke_.color_; }

        void setGCSettings(GraphicContext& gc) const noexcept;

        void setByXMLElement(tinyxml2::XMLElement* xml_element) noexcept;;
        void setDefault() noexcept;

        ErrorCode parseTransform(const char* str) noexcept;


        void updateNumericAttr(AttrID attr_id, SVGNumericAttr* out_attr) noexcept;
        void updateColorAttr(AttrID attr_id, SVGColorAttr* out_attr) noexcept;
    };

} // End of namespace

#endif // GrainSVGPaintStyle_hpp