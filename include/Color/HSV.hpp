//
//  HSV.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainHSV_hpp
#define GrainHSV_hpp

#include "Grain.hpp"
#include "Color/Color.hpp"


namespace Grain {

    class RGB;
    class HSL;
    class YUV;
    class CIEXYZ;
    class CIExyY;


    /**
     *  @brief HSV Color.
     *
     *  An HSV color is defined by three attributes:
     *  - Hue: The color's shade on the color wheel.
     *  - Saturation: The intensity or vibrancy of the color.
     *  - Value: The brightness of the color.
     *
     *  Each attribute ranges from 0 to 1. The HSV model is widely used for
     *  intuitive color selection and manipulation in graphics and design.
     */
    class HSV {
    public:
        HSV() noexcept {}
        HSV(float h, float s, float v) noexcept {
            m_data[0] = h - static_cast<int32_t>(h);
            m_data[1] = s;
            m_data[2] = v;
        }
        HSV(const RGB& rgb) noexcept { set(rgb); }
        HSV(const YUV& yuv, Color::Space yuv_color_space) noexcept { set(yuv, yuv_color_space); }

        HSV(const CIEXYZ& xyz) noexcept {
            set(xyz);
        }

        HSV(const CIExyY& xyY) noexcept {
            set(xyY);
        }

        HSV(const char* csv) noexcept { set(csv); }

        ~HSV() noexcept {}


        virtual const char* className() const noexcept { return "HSV"; }

        friend std::ostream& operator << (std::ostream& os, const HSV* o) {
            o == nullptr ? os << "HSV nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os,const  HSV& o) {
            return os << o.m_data[0] << ", " << o.m_data[1] << ", " << o.m_data[2];
        }


        HSV& operator = (const RGB& v);

        bool operator == (const HSV& v) const {
            return m_data[0] == v.m_data[0] && m_data[1] == v.m_data[1] && m_data[2] == v.m_data[2];
        }

        bool operator != (const HSV& v) const {
            return m_data[0] != v.m_data[0] || m_data[1] != v.m_data[1] || m_data[2] != v.m_data[2];
        }


        float hue() const noexcept { return m_data[0]; }
        float saturation() const noexcept { return m_data[1]; }
        float value() const noexcept { return m_data[2]; }

        float* mutDataPtr() noexcept { return m_data; }
        const float* dataPtr() const noexcept { return m_data; }

        bool isSame(const HSV& hsv) const noexcept {
            return (std::fabs(m_data[0] - hsv.m_data[0]) < std::numeric_limits<float>::min() &&
                    std::fabs(m_data[1] - hsv.m_data[1]) < std::numeric_limits<float>::min() &&
                    std::fabs(m_data[2] - hsv.m_data[2]) < std::numeric_limits<float>::min());
        }


        void set(float h, float s, float v) noexcept {
            m_data[0] = h - static_cast<int32_t>(h);
            m_data[1] = s;
            m_data[2] = v; }

        void set(const float* comp) noexcept {
            if (comp != nullptr) {
                m_data[0] = comp[0];
                m_data[1] = comp[1];
                m_data[2] = comp[2];
            }
        }

        void setHue(float h) noexcept { m_data[0] = h - std::floor(h); }
        void setSaturation(float s) noexcept { m_data[1] = s; }
        void setValue(float v) noexcept { m_data[2] = v; }

        void set(const RGB& rgb) noexcept;
        void set(const HSL& hsl) noexcept;
        void set(const YUV& yuv, Color::Space yuv_color_space) noexcept;
        void set(const CIEXYZ& xyz) noexcept;
        void set(const CIExyY& xyY) noexcept;
        void setRGB(float r, float g, float b) noexcept;

        void set(const char* csv) noexcept;


        //

        void addHue(float v) noexcept { setHue(m_data[0] + v); }
        void mulSaturation(float f) noexcept { m_data[1] *= f; }
        void mulValue(float f) noexcept { m_data[2] *= f; }

        void rotateHue(float angle) {
            m_data[0] = fmod(m_data[0] + angle / 360.0f, 1.0);
            if (m_data[0] < 0.0f) {
                m_data[0] += 360.0f;
            }
        }

        HSV blend(const HSV& hsv, float t) noexcept {
            if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
            float ti = 1.0f - t;
            HSV result;
            result.m_data[0] = m_data[0] * ti + hsv.m_data[0] * t;
            result.m_data[1] = m_data[1] * ti + hsv.m_data[1] * t;
            result.m_data[2] = m_data[2] * ti + hsv.m_data[2] * t;
            return result;
        }

    public:
        float m_data[3]{};
    };


} // End of namespace Grain

#endif // GrainHSV_hpp
