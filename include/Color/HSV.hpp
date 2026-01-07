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
            data_[0] = h - static_cast<int32_t>(h);
            data_[1] = s;
            data_[2] = v;
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
            return os << o.data_[0] << ", " << o.data_[1] << ", " << o.data_[2];
        }


        HSV& operator = (const RGB& v);

        bool operator == (const HSV& v) const {
            return data_[0] == v.data_[0] && data_[1] == v.data_[1] && data_[2] == v.data_[2];
        }

        bool operator != (const HSV& v) const {
            return data_[0] != v.data_[0] || data_[1] != v.data_[1] || data_[2] != v.data_[2];
        }


        float hue() const noexcept { return data_[0]; }
        float saturation() const noexcept { return data_[1]; }
        float value() const noexcept { return data_[2]; }

        float* mutDataPtr() noexcept { return data_; }
        const float* dataPtr() const noexcept { return data_; }

        bool isSame(const HSV& hsv) const noexcept {
            return (std::fabs(data_[0] - hsv.data_[0]) < std::numeric_limits<float>::min() &&
                    std::fabs(data_[1] - hsv.data_[1]) < std::numeric_limits<float>::min() &&
                    std::fabs(data_[2] - hsv.data_[2]) < std::numeric_limits<float>::min());
        }


        void set(float h, float s, float v) noexcept {
            data_[0] = h - static_cast<int32_t>(h);
            data_[1] = s;
            data_[2] = v; }

        void set(const float* comp) noexcept {
            if (comp != nullptr) {
                data_[0] = comp[0];
                data_[1] = comp[1];
                data_[2] = comp[2];
            }
        }

        void setHue(float h) noexcept { data_[0] = h - std::floor(h); }
        void setSaturation(float s) noexcept { data_[1] = s; }
        void setValue(float v) noexcept { data_[2] = v; }

        void set(const RGB& rgb) noexcept;
        void set(const HSL& hsl) noexcept;
        void set(const YUV& yuv, Color::Space yuv_color_space) noexcept;
        void set(const CIEXYZ& xyz) noexcept;
        void set(const CIExyY& xyY) noexcept;
        void setRGB(float r, float g, float b) noexcept;

        void set(const char* csv) noexcept;


        //

        void addHue(float v) noexcept { setHue(data_[0] + v); }
        void mulSaturation(float f) noexcept { data_[1] *= f; }
        void mulValue(float f) noexcept { data_[2] *= f; }

        void rotateHue(float angle) {
            data_[0] = fmod(data_[0] + angle / 360.0f, 1.0);
            if (data_[0] < 0.0f) {
                data_[0] += 360.0f;
            }
        }

        HSV blend(const HSV& hsv, float t) noexcept {
            if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
            float ti = 1.0f - t;
            HSV result;
            result.data_[0] = data_[0] * ti + hsv.data_[0] * t;
            result.data_[1] = data_[1] * ti + hsv.data_[1] * t;
            result.data_[2] = data_[2] * ti + hsv.data_[2] * t;
            return result;
        }

    public:
        float data_[3]{};
    };


} // End of namespace Grain

#endif // GrainHSV_hpp
