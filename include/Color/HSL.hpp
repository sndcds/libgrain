//
//  HSL.hpp
//
//  Created by Roald Christesen on from 30.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainHSL_hpp
#define GrainHSL_hpp

#include "Grain.hpp"
#include "Type/Type.hpp"
#include "Color/Color.hpp"


namespace Grain {

    class RGB;
    class HSV;
    class YUV;
    class CIEXYZ;
    class CIExyY;


    /**
     *  @brief HSL Color.
     *
     *  An HSL color is defined by three attributes:
     *  - Hue: The color's shade on the color wheel, measured in degrees [0, 360].
     *  - Saturation: The intensity or vibrancy of the color, measured as a fraction [0, 1].
     *  - Lightness: The brightness of the color, measured as a fraction [0, 1].
     *
     *  The HSL model is widely used for intuitive color selection and manipulation
     *  in graphics and design due to its user-friendly representation of colors.
     */
    class HSL {

    public:
        float m_data[3] = { 0.0f, 0.0f, 0.0f };

    public:
        HSL() noexcept = default;
        HSL(float h, float s, float l) noexcept {
            m_data[0] = Type::wrappedValue<float>(h, 0.0f, 1.0f);
            m_data[1] = s;
            m_data[2] = l;
        }
        explicit HSL(const RGB& rgb) noexcept { set(rgb); }
        explicit HSL(const HSV &hsv) noexcept { set(hsv); }
        explicit HSL(const YUV &yuv, Color::Space yuv_color_space) noexcept {
            set(yuv, yuv_color_space);
        }
        explicit HSL(const CIEXYZ &xyz) noexcept {
            set(xyz);
        }
        explicit HSL(const CIExyY &xyY) noexcept {
            set(xyY);
        }
        explicit HSL(const char* csv, char delimiter = ',') noexcept {
            setByCSV(csv, delimiter);
        }

        ~HSL() noexcept = default;


        [[nodiscard]] virtual const char* className() const noexcept { return "HSL"; }

        friend std::ostream& operator << (std::ostream &os, const HSL* o) {
            o == nullptr ? os << "HSL nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream &os, const HSL &o) {
            os << o.m_data[0] << ", " << o.m_data[1] << ", " << o.m_data[2];
            return os;
        }

        // Operator overloading
        HSL& operator = (const RGB& v);

        bool operator == (const HSL &v) const {
            return m_data[0] == v.m_data[0] && m_data[1] == v.m_data[1] && m_data[2] == v.m_data[2];
        }

        bool operator != (const HSL &v) const {
            return m_data[0] != v.m_data[0] || m_data[1] != v.m_data[1] || m_data[2] != v.m_data[2];
        }

        // Get
        [[nodiscard]] float hue() const noexcept { return m_data[0]; }
        [[nodiscard]] float saturation() const noexcept { return m_data[1]; }
        [[nodiscard]] float lightness() const noexcept { return m_data[2]; }

        [[nodiscard]] float* mutDataPtr() noexcept { return m_data; }
        [[nodiscard]] const float* dataPtr() const noexcept { return m_data; }

        [[nodiscard]] bool isSame(const HSL &hsl) const noexcept {
            return (std::fabs(m_data[0] - hsl.m_data[0]) < std::numeric_limits<float>::min() &&
                    std::fabs(m_data[1] - hsl.m_data[1]) < std::numeric_limits<float>::min() &&
                    std::fabs(m_data[2] - hsl.m_data[2]) < std::numeric_limits<float>::min());
        }

        // Set
        void set(float h, float s, float v) noexcept {
            m_data[0] = Type::wrappedValue<float>(h, 0.0f, 1.0f);
            m_data[1] = s;
            m_data[2] = v;
        }

        void set(const float* comp) noexcept {
            if (comp != nullptr) {
                m_data[0] = comp[0];
                m_data[1] = comp[1];
                m_data[2] = comp[2];
            }
        }

        void setHue(float h) noexcept { m_data[0] = Type::wrappedValue<float>(h, 0.0f, 1.0f); }
        void setSaturation(float s) noexcept { m_data[1] = s; }
        void setValue(float v) noexcept { m_data[2] = v; }

        void set(const RGB &rgb) noexcept;
        void set(const HSV &hsv) noexcept;
        void set(const YUV &yuv, Color::Space yuv_color_space) noexcept;
        void set(const CIEXYZ &xyz) noexcept;
        void set(const CIExyY &xyY) noexcept;
        void setRGB(float r, float g, float b) noexcept;

        bool setByCSV(const char* csv, char delimiter = ',') noexcept;


        //

        void addHue(float v) noexcept { setHue(m_data[0] + v); }
        void mulSaturation(float f) noexcept { m_data[1] *= f; }
        void mulValue(float f) noexcept { m_data[2] *= f; }

        void rotateHue(float angle) noexcept { setHue(m_data[0] + angle / 360); }

        [[nodiscard]] HSL blend(const HSL &hsl, float t) noexcept {
            if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
            float ti = 1.0f - t;
            HSL result;
            result.m_data[0] = m_data[0] * ti + hsl.m_data[0] * t;
            result.m_data[1] = m_data[1] * ti + hsl.m_data[1] * t;
            result.m_data[2] = m_data[2] * ti + hsl.m_data[2] * t;
            return result;
        }
    };


} // End of namespace Grain

#endif // GrainHSL_hpp
