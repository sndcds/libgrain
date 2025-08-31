//
//  CSS.hpp
//
//  Created by Roald Christesen on from 31.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 12.07.2025
//

#ifndef GrainCSS_hpp
#define GrainCSS_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Geometry.hpp"
#include "Type/Fix.hpp"
#include "String/String.hpp"


namespace Grain {

    enum class CSSUnitContext {
        Undefined = -1,
        Absolute = 0,       ///< Absolute units represent fixed, physical sizes and are not influenced by other elements or the environment.
        Relative,           ///< Relative units depend on the context, such as the size of a parent element, the root element, or the viewport.
        Time,
        Angle,
        Frequency,
        Resolution,
        Percentage          ///< Relative to another property, such as the parent element's width, height, or specific property context.
    };


    /**
     *  @brief CSS units.
     *
     *  @note Any changes here must reflect in `CSS::_g_css_units[]`.
     */
    enum class CSSUnit : int32_t {

        Undefined = 0,
        Absolute = 1,

        Millimeter,         ///< mm
        Centimeter,         ///< cm
        QuarterMillimeter,  ///< q
        Inch,               ///< in
        Pixel,              ///< px
        Point,              ///< pt
        Pica,               ///< pc

        Relative_em,
        Relative_rem,
        Relative_ex,
        Relative_ch,
        Relative_lh,
        Relative_rlh,

        Viewport_vw,
        Viewport_vh,
        Viewport_vmin,
        Viewport_vmax,
        Viewport_svw,
        Viewport_svh,
        Viewport_lvw,
        Viewport_lvh,
        Viewport_dvw,
        Viewport_dvh,

        Container_cqw,
        Container_cqh,
        Container_cqmin,
        Container_cqmax,

        Time_s,
        Time_ms,

        Angle_deg,
        Angle_grad,
        Angle_rad,
        Angle_turn,

        Frequency_Hz,
        Frequency_kHz,

        Resolution_dpi,
        Resolution_dpcm,
        Resolution_dppx,

        Percentage,

        First = Undefined,
        Last = Percentage,

        FirstAbsolute = Absolute,
        LastAbsolute = Pica,
        FirstRelativ = Relative_em,
        LastRelative = Relative_rlh,
        FirstViewport = Viewport_vw,
        LastViewport = Viewport_dvh,
        FirstContainer = Container_cqw,
        LastContainer = Container_cqmax,
        FirstTime = Time_s,
        LastTime = Time_ms,
        FirstAngle = Angle_deg,
        LastAngle = Angle_turn,
        FirstFrequency = Frequency_Hz,
        LastFrequency = Frequency_kHz,
        FirstResolution = Resolution_dpi,
        LastResolution = Resolution_dppx
    };


    /**
     *  @brief Enumerates all major and proposed CSS color functions.
     *
     *  This enum represents the various CSS color functions defined in the
     *  CSS Color Module Level 4 and Level 5 specifications. These functions
     *  allow defining colors in different color models and formats.
     *
     *  @details
     *  The following color functions are included:
     *
     *  - **RGB**: Specifies a color using red, green, and blue components (0-255 or percentages).
     *  - **RGBA**: (Legacy) Same as RGB, but explicitly includes an alpha (opacity) value.
     *  - **HSL**: Represents colors using hue (degrees), saturation (%), and lightness (%).
     *  - **HSLA**: (Legacy) Same as HSL, but explicitly includes an alpha (opacity) value.
     *  - **HWB**: Uses hue, whiteness (%), and blackness (%) to specify a color.
     *  - **CMYK**: Represents colors in the cyan, magenta, yellow, and key (black) model. *(Proposed in CSS Level 5)*.
     *  - **Lab**: A perceptually uniform color space with parameters for lightness, a*, and b* chroma components.
     *  - **Lch**: Similar to Lab but uses cylindrical coordinates for chroma and hue.
     *  - **Color**: Specifies colors in named or device-specific color spaces like `srgb`, `display-p3`, `rec2020`, etc.
     *  - **Gray**: Represents grayscale colors using a single value for lightness. *(Proposed in CSS Level 5)*.
     *  - **OKLCh**: A modern variant of Lch with better perceptual uniformity (OKLAB color space).
     *  - **OKLab**: A modern variant of Lab with better perceptual uniformity (OKLAB color space).
     *
     *  @note
     *  Functions such as `RGBA` and `HSLA` are considered legacy, as modern syntax
     *  supports alpha transparency directly in `RGB` and `HSL`.
     *
     *  @see https://drafts.csswg.org/css-color/ for the latest CSS Color Module specifications.
     */
    enum class CSSColorFunction {
        Undefined = 0,
        RGB,
        RGBA,
        HSL,
        HSLA,
        HWB,
        CMYK,
        Lab,
        Lch,
        Color,
        Gray,
        OKLCh,
        OKLab
    };


    typedef struct {
        const char* m_unit_str;
        int32_t m_unit_str_len;
        CSSUnit m_unit;
        CSSUnitContext m_unit_context;
    } CSSUnitInfo;


    class CSSValue {

    protected:
        CSSUnit m_unit = CSSUnit::Undefined;    ///< The CSSUnit of the vlaue.
        bool m_is_float = false;                ///< Flag indicating whether the number is an integer or a floating point number.
        Fix m_value = 0;                        ///< The value represented as Fix.
        bool m_valid = false;                   ///< Flag indicating wether the value is valid and can be used.

    public:
        CSSValue() = default;

        CSSValue(Fix value, CSSUnit unit) {
            m_value = value;
            m_unit = unit;
            m_valid = m_unit != CSSUnit::Undefined;
        }

        virtual const char* className() const noexcept { return "CSSValue"; }

        friend std::ostream& operator << (std::ostream &os, const CSSValue* o) {
            o == nullptr ? os << "CSSValue nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream &os, const CSSValue &o) {
            if (o.m_valid) {
                os << o.m_value << " " << o.unitName();
            }
            else {
                os << "invalid css value";
            }
            return os;
        }


        bool validate() noexcept {
            m_valid = m_unit != CSSUnit::Undefined;
            return m_valid;
        }

        void setDouble(double value, CSSUnit unit) noexcept {
            m_value.setDouble(value);
            m_unit = unit;
            validate();
        }

        void setDoubleAbsolut(double value) noexcept {
            setDouble(value, CSSUnit::Absolute);
        }

        void setInt32(int32_t value, CSSUnit unit) noexcept {
            m_value.setInt32(value);
            m_unit = unit;
            validate();
        }

        void setInt32Absolute(int32_t value) noexcept {
            setInt32(value, CSSUnit::Absolute);
        }

        void setIsFloat(bool is_float) noexcept { m_is_float = is_float; }


        void undef() noexcept {
            m_value = 0.0;
            m_unit = CSSUnit::Undefined;
            m_is_float = false;
            m_valid = false;
        }

        CSSUnit unit() const noexcept { return m_unit; }
        const char* unitName() const noexcept;

        bool isColorLevelUnit() const noexcept { return m_unit == CSSUnit::Absolute || m_unit <= CSSUnit::Percentage; }
        bool isAngleUnit() const noexcept { return m_unit == CSSUnit::Absolute || (m_unit >= CSSUnit::FirstAngle && m_unit <= CSSUnit::LastAngle); }
        bool isWithoutUnitOrPercentage() const noexcept { return m_unit == CSSUnit::Absolute && m_unit <= CSSUnit::Percentage; }
        bool isPercentage() const noexcept { return m_unit >= CSSUnit::Percentage; }

        /* TODO: ????
        bool isAbsolute() const noexcept { return m_unit >= CSSUnit::FirstAbsolute && m_unit <= CSSUnit::LastAbsolute; }
        bool isRelative() const noexcept { return m_unit >= CSSUnit::FirstRelativ && m_unit <= CSSUnit::LastRelative; }
        bool isViewport() const noexcept { return m_unit >= CSSUnit::FirstViewport && m_unit <= CSSUnit::LastViewport; }
        bool isContainer() const noexcept { return m_unit >= CSSUnit::FirstContainer && m_unit <= CSSUnit::LastContainer; }
        bool isTime() const noexcept { return m_unit >= CSSUnit::FirstTime && m_unit <= CSSUnit::LastTime; }
        bool isAngle() const noexcept { return m_unit >= CSSUnit::FirstAngle && m_unit <= CSSUnit::LastAngle; }
        bool isFrequency() const noexcept { return m_unit >= CSSUnit::FirstFrequency && m_unit <= CSSUnit::LastFrequency; }
        bool isResolution() const noexcept { return m_unit >= CSSUnit::FirstResolution && m_unit <= CSSUnit::LastResolution; }
         */

        Fix value() const noexcept { return m_value; }
        float valueAsFloat() const noexcept { return m_value.asFloat(); }
        double valueAsDouble() const noexcept { return m_value.asDouble(); }
        int32_t valueAsInt32() const noexcept { return m_value.asInt32(); }
        double valueAsDoubleConsiderPercentage() const noexcept;
        double valueForColorLevel() const noexcept;
        double valueForAngleDegree() const noexcept;

        double valueSVGPixel(double dpi = 96) const noexcept;


        bool isValid() const noexcept { return m_valid; }
    };


    class CSSValidator {

        friend class CSS;

    protected:

        int32_t m_value_n = 0;
        int32_t m_comma_n = 0;
        int32_t m_slash_n = 0;
        int32_t m_pre_slash_value_n;

    public:
        friend std::ostream& operator << (std::ostream &os, const CSSValidator* o) {
            o == nullptr ? os << "CSSValidator nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream &os, const CSSValidator &o) {
            os << "m_value_n: " << o.m_value_n << '\n';
            os << "m_comma_n: " << o.m_comma_n << '\n';
            os << "m_slash_n: " << o.m_slash_n << '\n';
            os << "m_pre_slash_value_n: " << o.m_pre_slash_value_n << '\n';
            return os;
        }

        bool checkValueContent(const char* str) noexcept;
    };


    class CSS : public Object {

    protected:
        static const CSSUnitInfo _g_css_unit_infos[];

    public:
        CSS() noexcept;
        ~CSS() noexcept;

        static int32_t extractValuesFromStr(const char* str, char* buffer, int32_t buffer_size, char closing_char, int32_t values_size, float* out_values);
        static ErrorCode extractCSSValueFromStr(const char* str, CSSValue& out_value, char** next_value_ptr) noexcept;
        static const char* unitName(CSSUnit unit) noexcept;

        static bool _css_strcmp(const char* a, const char* b) noexcept;
        static bool _css_isDelimiter(const char c) noexcept;
        static bool _css_check_comma_delimiters(const char* str) noexcept;
    };


} // End of namespace Grain

#endif // GrainCSS_hpp
