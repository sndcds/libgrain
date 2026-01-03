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
        float alpha_ = 1.0f;

    public:
        RGBA() noexcept : RGB(), alpha_(1.0f) {}
        RGBA(float r, float g, float b, float alpha) noexcept : RGB(r, g, b), alpha_(alpha) {}
        explicit RGBA(float value) noexcept : RGB(value), alpha_(1.0f) {}
        explicit RGBA(uint32_t value) noexcept { set32bit(value); }
        explicit RGBA(float r, float g, float b) noexcept : RGB(r, g, b), alpha_(1.0f) {}
        explicit RGBA(const RGB& rgb, float alpha) noexcept : RGB(rgb), alpha_(alpha) {}
        explicit RGBA(const RGBA& a, const RGBA& b, float blend) noexcept { setBlend(a, b, blend); }
        explicit RGBA(const RGB& a, const RGBA& b, float blend) noexcept { setBlend(RGBA(a, 1.0f), b, blend); }
        explicit RGBA(const RGBA& a, const RGB& b, float blend) noexcept { setBlend(a, RGBA(b, 1.0f), blend); }
        explicit RGBA(const HSV& hsv, float alpha) noexcept : RGB(hsv), alpha_(alpha) {}

        RGBA(int32_t r, int32_t g, int32_t b, int32_t a, int32_t max) noexcept {
            float f = 1.0f / static_cast<float>(max);
            data_[0] = f * static_cast<float>(r);
            data_[1] = f * static_cast<float>(g);
            data_[2] = f * static_cast<float>(b);
            alpha_ = f * static_cast<float>(a);
        }
        explicit RGBA(const String& csv) noexcept;
        explicit RGBA(const char* csv) noexcept;

        ~RGBA() noexcept override = default;


        [[nodiscard]] const char* className() const noexcept override { return "RGBA"; }

        friend std::ostream& operator << (std::ostream& os, const RGBA* o) {
            o == nullptr ? os << "RGBA nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const RGBA& o) {
            os << o.data_[0] << ", " << o.data_[1] << ", " << o.data_[2] << ", " << o.alpha_;
            return os;
        }

        RGBA& operator = (const RGBA&) = default;

        bool operator == (const RGBA& v) const {
            return data_[0] == v.data_[0] && data_[1] == v.data_[1] && data_[2] == v.data_[2]  && alpha_ == v.alpha_;
        }

        bool operator != (const RGBA& v) const {
            return data_[0] != v.data_[0] || data_[1] != v.data_[1] || data_[2] != v.data_[2] || alpha_ != v.alpha_;
        }

        [[nodiscard]] float alpha() const noexcept { return alpha_; }
        [[nodiscard]] uint32_t rgba32bit() const noexcept;
        void values(float* out_values) const noexcept override;

        [[nodiscard]] bool isSame(const RGBA& rgba, float tolerance = 0.0001f) const noexcept;

        void black() noexcept override { data_[0] = data_[1] = data_[2] = 0.0f; alpha_ = 1.0f; }
        void white() noexcept override { data_[0] = data_[1] = data_[2] = 1.0f; alpha_ = 1.0f;  }

        void setGrey(float value) noexcept override { data_[0] = data_[1] = data_[2] = value; alpha_ = 1.0f; }
        void setRGB(const RGB& color) {
            data_[0] = color.data_[0]; data_[1] = color.data_[1]; data_[2] = color.data_[2]; alpha_ = 1.0f;
        }
        void setRGBA(const RGB& color, float alpha) {
            data_[0] = color.data_[0]; data_[1] = color.data_[1]; data_[2] = color.data_[2]; alpha_ = alpha;
        }
        void setRGB(float r, float g, float b) { data_[0] = r; data_[1] = g; data_[2] = b; alpha_ = 1.0f; }
        void setRGBA(float r, float g, float b, float alpha) { data_[0] = r; data_[1] = g; data_[2] = b; alpha_ = alpha; }

        void setLerp(const RGBA& a, const RGBA& b, double t) noexcept {
            data_[0] = a.data_[0] + t * (b.data_[0] - a.data_[0]);
            data_[1] = a.data_[1] + t * (b.data_[1] - a.data_[1]);
            data_[2] = a.data_[2] + t * (b.data_[2] - a.data_[2]);
            alpha_ = a.alpha_ + t * (b.alpha_ - a.alpha_);
        }

        void set32bit(uint32_t value) noexcept {
            data_[0] = static_cast<float>((value & 0xFF000000) >> 24) / 255;
            data_[1] = static_cast<float>((value & 0xFF0000) >> 16) / 255;
            data_[2] = static_cast<float>((value & 0xFF00) >> 8) / 255;
            alpha_ = static_cast<float>(value & 0xFF) / 255;
        }

        int32_t setByCSV(const char* csv) noexcept override;

        ErrorCode setByCSS(const char* css_str) noexcept;

        void setValues(const float* comp) noexcept override;
        void setValues(const float* comp, float scale) noexcept override;


        void setAlpha(float alpha) noexcept { alpha_ = alpha; }
        void setBlend(const RGBA& a, float t) noexcept;
        void setBlend(const RGBA& a, const RGBA& b, float t) noexcept;
        void mixbox(const RGBA& color1, const RGBA& color2, float t) noexcept;

        void scale(float scale) noexcept override;
    };


} // End of namespace Grain

#endif // GrainRGBA_hpp
