//
//  RGB.hpp
//
//  Created by Roald Christesen on from 17.04.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainRGBA_hpp
#define GrainRGBA_hpp

#include "Color/RGB.hpp"


namespace Grain {

    class HSV;


    /**
     *  @brief GrRGBA Color Description.
     *
     *  An RGBA color is defined by its red, green, blue and alpha components. Each of these components is defined within the range of 0 to 1, representing the intensity or transparency of that color channel.
     *  The RGBA color model is an extension of the RGB model, with the addition of the alpha channel.
     *
     *  The alpha component defines the opacity of the color. It has a value between 0 and 1, where 0 represents complete transparency (fully transparent), and 1 represents complete opacity (fully setOpaque).
     *  The alpha channel allows you to control how an object with this color is blended with other objects when composing an image.
     *
     *  RGBA colors are widely used in computer graphics, image processing, and user interfaces. The alpha component is particularly valuable for creating soft edges, fading effects, and overlaying objects with variable transparency.
     */
    class RGBA : public RGB {

    public:
        static const RGBA kBlack;
        static const RGBA kWhite;

    public:
        float m_alpha = 1.0f;

    public:
        RGBA() noexcept : RGB(), m_alpha(1.0f) {}
        explicit RGBA(float value) noexcept : RGB(value), m_alpha(1.0f) {}
        explicit RGBA(uint32_t value) noexcept { set32bit(value); }
        RGBA(float r, float g, float b) noexcept : RGB(r, g, b), m_alpha(1.0f) {}
        RGBA(float r, float g, float b, float alpha) noexcept : RGB(r, g, b), m_alpha(alpha) {}
        RGBA(const RGB& rgb, float alpha) noexcept : RGB(rgb), m_alpha(alpha) {}
        RGBA(const RGBA& a, const RGBA& b, float blend) noexcept { setBlend(a, b, blend); }
        RGBA(const RGB& a, const RGBA& b, float blend) noexcept { setBlend(RGBA(a, 1.0f), b, blend); }
        RGBA(const RGBA& a, const RGB& b, float blend) noexcept { setBlend(a, RGBA(b, 1.0f), blend); }
        RGBA(const HSV& hsv, float alpha) noexcept : RGB(hsv), m_alpha(alpha) {}

        RGBA(int32_t r, int32_t g, int32_t b, int32_t a, int32_t max) noexcept {
            float f = 1.0f / static_cast<float>(max);
            m_data[0] = f * static_cast<float>(r);
            m_data[1] = f * static_cast<float>(g);
            m_data[2] = f * static_cast<float>(b);
            m_alpha = f * static_cast<float>(a);
        }
        explicit RGBA(const String& csv) noexcept;
        explicit RGBA(const char* csv) noexcept;

        ~RGBA() noexcept = default;


        [[nodiscard]] const char* className() const noexcept override { return "RGBA"; }

        friend std::ostream& operator << (std::ostream& os, const RGBA* o) {
            o == nullptr ? os << "RGBA nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const RGBA& o) {
            os << o.m_data[0] << ", " << o.m_data[1] << ", " << o.m_data[2] << ", " << o.m_alpha;
            return os;
        }


        // Operator overloading, hides RGB::operator = (that is ok and can be ignored)
        RGBA& operator = (const RGB& v) {
            m_data[0] = v.m_data[0];
            m_data[1] = v.m_data[1];
            m_data[2] = v.m_data[2];
            return *this;
        }


        bool operator == (const RGBA& v) const {
            return m_data[0] == v.m_data[0] && m_data[1] == v.m_data[1] && m_data[2] == v.m_data[2]  && m_alpha == v.m_alpha;
        }

        bool operator != (const RGBA& v) const {
            return m_data[0] != v.m_data[0] || m_data[1] != v.m_data[1] || m_data[2] != v.m_data[2] || m_alpha != v.m_alpha;
        }

        [[nodiscard]] float alpha() const noexcept { return m_alpha; }
        [[nodiscard]] uint32_t rgba32bit() const noexcept;
        void values(float* out_values) const noexcept override;

        [[nodiscard]] bool isSame(const RGBA& rgba, float tolerance = 0.0001f) const noexcept;

        void black() noexcept override { m_data[0] = m_data[1] = m_data[2] = 0.0f; m_alpha = 1.0f; }
        void white() noexcept override { m_data[0] = m_data[1] = m_data[2] = 1.0f; m_alpha = 1.0f;  }

        void setGrey(float value) noexcept override { m_data[0] = m_data[1] = m_data[2] = value; m_alpha = 1.0f; }
        void setRGB(const RGB& color) {
            m_data[0] = color.m_data[0]; m_data[1] = color.m_data[1]; m_data[2] = color.m_data[2]; m_alpha = 1.0f;
        }
        void setRGBA(const RGB& color, float alpha) {
            m_data[0] = color.m_data[0]; m_data[1] = color.m_data[1]; m_data[2] = color.m_data[2]; m_alpha = alpha;
        }
        void setRGB(float r, float g, float b) { m_data[0] = r; m_data[1] = g; m_data[2] = b; m_alpha = 1.0f; }
        void setRGBA(float r, float g, float b, float alpha) { m_data[0] = r; m_data[1] = g; m_data[2] = b; m_alpha = alpha; }

        void set32bit(uint32_t value) noexcept {
            m_data[0] = (float)((value & 0xFF000000) >> 24) / 255;
            m_data[1] = (float)((value & 0xFF0000) >> 16) / 255;
            m_data[2] = (float)((value & 0xFF00) >> 8) / 255;
            m_alpha = (float)(value & 0xFF) / 255;
        }

        int32_t setByCSV(const char* csv) noexcept override;

        ErrorCode setByCSS(const char* css_str) noexcept;

        void setValues(const float* comp) noexcept override;
        void setValues(const float* comp, float scale) noexcept override;


        void setAlpha(float alpha) noexcept { m_alpha = alpha; }
        void setBlend(const RGBA& a, const RGBA& b, float t) noexcept;
        void mixbox(const RGBA& color1, const RGBA& color2, float t) noexcept;

        void scale(float scale) noexcept override;
    };


} // End of namespace Grain

#endif // GrainRGBA_hpp
